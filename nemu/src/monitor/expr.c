#include "nemu.h"
#include "cpu/reg.h"
#include "memory/memory.h"

#include <stdlib.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum
{
	NOTYPE = 256,
	EQ,
	NUM,
	REG,
	HEX,
	AND,
	NOTEQUAL,
	DEREF,
	NEG,
	SYMB

	/* TODO: Add more token types */

};

static struct rule
{
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +", NOTYPE},				// white space
	{"\\+", '+'},				// plus
	{"==", EQ},					// equal
	{"-", '-'},					// minus
	{"/", '/'},					// divide
	{"\\*", '*'},				// multi
	{"\\(", '('},				// parenthess_l
	{"\\)", ')'},				// parenthess_r
	{"0[Xx][a-fA-F0-9]+", HEX}, // hex
	{"[0-9]+", NUM},			// decimal
	{"\\$[a-z][A-Z]+", REG},	// register
	{"&&", AND},				// and
	{"!=", NOTEQUAL}			// not equal
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))
#define TOKEN_MAX_SIZE 32

static regex_t re[NR_REGEX];

/* Rules are used for more times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i++)
	{
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0)
		{
			regerror(ret, &re[i], error_msg, 128);
			assert(ret != 0);
		}
	}
}

typedef struct token
{
	int type;
	char str[32];
} Token;

Token tokens[32]; //按顺序存放已经识别出的token信息
int nr_token;	  //已经识别出的token数目

static bool make_token(char *e)
{
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;
	memset(tokens, 0, sizeof(tokens));

	while (e[position] != '\0')
	{
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i++)
		{
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
			{
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				printf("match regex[%d] at position %d with len %d: %.*s", i, position, substr_len, substr_len, substr_start);
				position += substr_len;

				if (substr_len > TOKEN_MAX_SIZE)
				{
					printf("%.*s Too long !! \n", substr_len, substr_start);
					return false;
				}

				if (nr_token >= TOKEN_MAX_SIZE)
				{
					printf("Too many tokens !!\n");
					return false;
				}

				/* TODO: Now a new token is recognized with rules[i].
				 * Add codes to perform some actions with this token.
				 */

				switch (rules[i].token_type)
				{
				case NOTYPE:
					break;
				case NUM:
				case HEX:
				case REG:
					strncpy(tokens[nr_token].str, substr_start, substr_len);
					tokens[nr_token].str[substr_len] = '\0';
				default:
					tokens[nr_token].type = rules[i].token_type;
					nr_token++;
				}

				break;
			}
		}

		if (i == NR_REGEX)
		{
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true;
}

static uint32_t get_val(int p)
{
	uint32_t val;
	if (tokens[p].type == NUM)
	{
		sscanf(tokens[p].str, "%d", &val);
	}
	else if (tokens[p].type == HEX)
	{
		sscanf(tokens[p].str, "%x", &val);
	}

	return val;
}

static bool is_opt(int opt)
{
	if (opt == '+' || opt == '-' || opt == '*' || opt == '/' || opt == EQ || opt == NOTEQUAL || opt == AND || opt == NEG)
		return true;
	else
		return false;
}

static int opt_level(int opt, int *cur)
{
	int level = 0;
	switch (opt)
	{
	case NEG:
		level = 2;
		break;
	case '+':
	case '-':
		level = 4;
		break;
	case '*':
	case '/':
		level = 3;
		break;
	case EQ:
	case NOTEQUAL:
		level = 7;
		break;
	case AND:
		level = 11;
		break;
	default:
		break;
	}

	if (level > *cur)
	{
		*cur = level;
		return 1;
	}
	else if (level < *cur)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

//4 + 3 * (2 - 1)
static int get_main_opt(int p, int q) {
  int parentheses = 0, cur = 0;
  int pos = -1;
  int i;
  for (i = p; i <= q; i++)
  {
      int type = tokens[i].type;
      if (type == '(')
      {
         parentheses++;
         continue;
      } else if (type == ')')
      {
          parentheses--;
          continue;
      }

      if (is_opt(type) && parentheses == 0)
      {
        //if opt and not surrounded by parentheses
        //find the highest level opt
        if (opt_level(type, &cur) > 0)
        {
          pos = i;
        }
        
      }
           
  }

  return pos;
  
}

static bool check_parentheses(int p, int q)
{
	int top = 0;
	bool is_first_matched = true;

	if (tokens[p].type != '(' || tokens[q].type != ')')
	{
		return false;
	}

	int i;
	for (i = p; i <= q; i++)
	{
		if (tokens[i].type == '(')
		{
			top++;
		}
		else if (tokens[i].type == ')')
		{
			top--;
			if (top == 0 && i != q)
			{
				is_first_matched = false;
			}
		}
	}

	if (top != 0 || is_first_matched == false)
	{
		return false;
	}
	else
	{
		return true;
	}
}

static uint32_t eval(int p, int q)
{
	if (p > q)
	{
		/* Bad expression */
		assert(0);
	}
	else if (p == q)
	{
		/* Single token.
		 * For now this token should be a number.
		 * Return the value of the number.
		 */
		uint32_t val = 0;
		if (tokens[p].type == NUM || tokens[p].type == HEX)
		{
			val = get_val(p);
		}
		else if (tokens[p].type == REG)
		{
			bool success = false;
			val = get_reg_val(tokens[p].str + 1, &success);
			assert(success == true);
		}

		return val;
	}
	else if (check_parentheses(p, q) == true)
	{
		/* The expression is surrounded by a matched pair of parentheses.
		 * If that is the case, just throw away the parentheses.
		 */
		return eval(p + 1, q - 1);
	}
	else
	{
		int32_t val1, val2;
		int main_opt = get_main_opt(p, q);

		if (tokens[main_opt].type == NEG)
		{
			return -eval(main_opt + 1, q);
		}
		else if (tokens[main_opt].type == DEREF)
		{
			return vaddr_read(eval(main_opt + 1, q), 0, 4);
		}

		val1 = eval(p, main_opt - 1);
		val2 = eval(main_opt + 1, q);
		switch (tokens[main_opt].type)
		{
		case '+':
			return val1 + val2;
		case '-':
			return val1 - val2;
		case '*':
			return val1 * val2;
		case '/':
			return val1 / val2;
		case EQ:
			return (uint32_t)(val1 == val2);
		case NOTEQUAL:
			return (uint32_t)(val1 != val2);
		case AND:
			return (uint32_t)(val1 && val2);
		default:
			assert(0);
			break;
		}
	}
}

uint32_t expr(char *e, bool *success)
{
	if (!make_token(e))
	{
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	int i;
	for (i = 0; i < nr_token; i++)
	{
		if (tokens[i].type == '*' && (i == 0 || is_opt(tokens[i - 1].type)))
		{
			tokens[i].type = DEREF;
		}
		if (tokens[i].type == '-' && (i == 0 || is_opt(tokens[i - 1].type)))
		{
			tokens[i].type = NEG;
		}
	}

	*success = true;

	return eval(0, nr_token - 1);
}