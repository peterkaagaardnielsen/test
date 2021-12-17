#include <ctype.h>
#include <string.h>
#include "stringutil.h"

int strncmp(const char *s1, const char *s2, size_t count)
{
  if (!count) return 0;

  while (--count && *s1 && *s1 == *s2)
  {
    s1++;
    s2++;
  }

  return *(unsigned char *) s1 - *(unsigned char *) s2;
}

int stricmp(const char *s1, const char *s2)
{
  char f, l;

  do 
  {
    f = ((*s1 <= 'Z') && (*s1 >= 'A')) ? *s1 + 'a' - 'A' : *s1;
    l = ((*s2 <= 'Z') && (*s2 >= 'A')) ? *s2 + 'a' - 'A' : *s2;
    s1++;
    s2++;
  } while ((f) && (f == l));

  return (int) (f - l);
}

int strnicmp(const char *s1, const char *s2, size_t count)
{
  int f, l;

  do 
  {
      if (((f = (unsigned char)(*(s1++))) >= 'A') && (f <= 'Z')) f -= 'A' - 'a';
      if (((l = (unsigned char)(*(s2++))) >= 'A') && (l <= 'Z')) l -= 'A' - 'a';
  } while (--count && f && (f == l));

  return f - l;
}

char *strchr(const char *s, int ch)
{
  while (*s && *s != (char) ch) s++;
  if (*s == (char) ch) return (char *) s;
  return NULL;
}

char *strrchr(const char *s, int ch)
{
  char *start = (char *) s;

  while (*s++);
  while (--s != start && *s != (char) ch);
  if (*s == (char) ch) return (char *) s;

  return NULL;
}

char *strstr(const char *str1, const char *str2)
{
  char *cp = (char *) str1;
  char *s1, *s2;

  if (!*str2) return (char *) str1;

  while (*cp)
  {
    s1 = cp;
    s2 = (char *) str2;

    while (*s1 && *s2 && !(*s1 - *s2)) s1++, s2++;
    if (!*s2) return cp;
    cp++;
  }

  return NULL;
}

char *stristr(const char *str1, const char *str2)
{
  char *cp = (char *) str1;
  char *s1, *s2;

  if (!*str2) return (char *) str1;

  while (*cp)
  {
    s1 = cp;
    s2 = (char *) str2;

    while (*s1 && *s2 && !(((*s1)|0x20) - ((*s2)|0x20))) s1++, s2++;
    if (!*s2) return cp;
    cp++;
  }

  return NULL;
}

size_t strspn(const char *string, const char *control)
{
  const  char *str = string;
  const  char *ctrl = control;

  unsigned char map[32];
  int count;

  // Clear out bit map
  for (count = 0; count < 32; count++) map[count] = 0;

  // Set bits in control map
  while (*ctrl)
  {
    map[*ctrl >> 3] |= (1 << (*ctrl & 7));
    ctrl++;
  }

  // 1st char NOT in control map stops search
  if (*str)
  {
    count = 0;
    while (map[*str >> 3] & (1 << (*str & 7)))
    {
      count++;
      str++;
    }
    
    return count;
  }

  return 0;
}

size_t strcspn(const char *string, const char *control)
{
  const  char *str = string;
  const  char *ctrl = control;

  unsigned char map[32];
  int count;

  // Clear out bit map
  for (count = 0; count < 32; count++) map[count] = 0;

  // Set bits in control map
  while (*ctrl)
  {
    map[*ctrl >> 3] |= (1 << (*ctrl & 7));
    ctrl++;
  }

  // 1st char in control map stops search
  count = 0;
  map[0] |= 1;
  while (!(map[*str >> 3] & (1 << (*str & 7))))
  {
    count++;
    str++;
  }
  return count;
}

char *strpbrk(const char *string, const char *control)
{
  const  char *str = string;
  const  char *ctrl = control;

  unsigned char map[32];
  int count;

  // Clear out bit map
  for (count = 0; count < 32; count++) map[count] = 0;

  // Set bits in control map
  while (*ctrl)
  {
    map[*ctrl >> 3] |= (1 << (*ctrl & 7));
    ctrl++;
  }

  // 1st char in control map stops search
  while (*str)
  {
    if (map[*str >> 3] & (1 << (*str & 7))) return (char *) str;
    str++;
  }

  return NULL;
}

char *strlwr(char *s)
{
  char *p = s;

  while (*p)
  {
    *p = (char) tolower(*p);
    p++;
  }

  return s;
}

char *strupr(char *s)
{
  char *p = s;

  while (*p)
  {
    *p = (char) toupper(*p);
    p++;
  }

  return s;
}


int strreplace(char *s1, const char s2, const char s3) {
  int cnt=0;

  char *p = s1;

  while (*p)
  {
    if (*p == s2) {
      *p=s3;
      cnt++;
    }
    p++;
  }

  return cnt;
}



