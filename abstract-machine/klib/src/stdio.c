#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define MAXF 7

#define type(lm, cs) (lm * 256 + cs)
enum {F_POUND = 0, F_ZERO, F_DASH, F_SPACE, F_PLUS, F_SQUOT, F_I};
enum {L_DH = 256 + 'h', L_DL = 256 + 'l'};
enum {int_t = type(0, 'd'), str_t = type(0, 's'), };
typedef struct convtspec {
  char fc[MAXF]; //flag character
  int fw;		   //field width
  int p;           //precision
  int lm;          //length modifier
  int cs;          //conversion specifision
} Convtspec;

void csfinit(Convtspec* csf) {
  for (int i = 0; i < MAXF; i++) csf->fc[i] = 0;
  csf->fw = csf->p = csf->lm = csf->cs = 0;
}

static inline const char* fparser(Convtspec* ptr, const char *c) {
  for (;;) {
    switch (*c) {
      case '\'':
	    if (*(c + 1) == ' ' && (*(c + 2) == '\'')) {
	      ptr->fc[F_SPACE] = 1; 
		  c = c + 2;
	    } else ptr->fc[F_SQUOT] = 1;
	    break;
	  case '#': ptr->fc[F_POUND] = 1; break;
      case '0': ptr->fc[F_ZERO] = 1; break;
	  case '-': ptr->fc[F_DASH] = 1; break;
	  case '+': ptr->fc[F_PLUS] = 1; break;
      case 'I': ptr->fc[F_I] = 1; break;
      default: return c;
    }
    ++c;
  }
  return c;
}

static inline const char* wparser(Convtspec* ptr, const char *c) {
  return c;
}

static inline const char* pparser(Convtspec* ptr, const char *c) {
  return c;
}

static inline const char* mparser(Convtspec* ptr, const char *c) {
  return c;
}

static inline const char* sparser(Convtspec* ptr, const char *c) {
  switch (*c) {
    case 'd': case 'i': ptr->cs = 'd'; break;
    case 's': ptr->cs = 's'; break;
	default: ptr->cs = 0; break; 
  }
  return ++c;
}

static struct parser {
  const char* (*funct)(Convtspec* ptr, const char *c);
} Parser[] = { 
 { fparser },  //flag parser
 { wparser },  //width parser
 { pparser },  //precision parser 
 { mparser },  //modifier parser 
 { sparser },  //specifision parser
};

static const char* parse(Convtspec* ptr, const char *p)  {
  for (int i = 0; i < sizeof(Parser) / sizeof(Parser[0]); i++)
    p = Parser[i].funct(ptr, p); //return the pointer pointing the next field.
  return p; 
}


#define CONVTF(buf, var, i) convt ## i (buf, cur, csf, var)
#define CASE(i, type) case i: CONVTF(buf, va_arg(*ap, type), i); break; 

static inline void convtint_t(char *buf, size_t* cur, Convtspec cs, int var) {
  /*TODO:flag precision and son on...*/
  int sign = 1;
  if (var < 0) { buf[(*cur)++] = '-'; sign = -1; }
  int cnt = 0; char num[20];
  do { num[cnt++] = sign * (var % 10) + '0'; var /= 10; } while (var != 0);
  while (cnt--) buf[(*cur)++] = num[cnt]; 
}

static inline void convtstr_t(char *buf, size_t* cur, Convtspec cs, char* var) {
  /*TODO*/ 
  for (; *var != '\0'; var++) buf[(*cur)++] = *var;
}

static const char* convt(char *buf, size_t* cur, const char* pchr, va_list* ap) {
  Convtspec csf;
  csfinit(&csf);
  const char* npchr = pchr + 1;
  if (*(npchr) == '\0') { 
    buf[(*cur)++] = '\0'; 
	return NULL;
  }

  npchr = parse(&csf, npchr);
  if (csf.cs == 0) {
    for (; pchr < npchr; pchr++) 
      buf[(*cur)++] = *pchr;   
  }
  else {
    switch (type(csf.lm, csf.cs)) {
      CASE(int_t, int) 	
	  CASE(str_t, char*)
	}
  }
  return npchr;
}


int printf(const char *fmt, ...) {
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  size_t cur = 0;
  const char* pchr = fmt;
  va_list aq; 
  va_copy(aq, ap);

  while (*pchr != '\0') { 
    if (*pchr != '%') { 
      out[cur++] = *pchr; 
	  pchr++; 
	}
	else {
	        pchr = convt(out, &cur, pchr, &aq); 
	  if (pchr == NULL) return -1; 
	}
  }
  va_end(aq);
  out[cur] = '\0';
  return cur;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  int cnt;
  va_start(ap, fmt);
  cnt = vsprintf(out, fmt, ap);
  va_end(ap);
  return cnt;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  return 0;
}

#endif
