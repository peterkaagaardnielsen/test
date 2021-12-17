int strncmp(const char *s1, const char *s2, size_t count);
int stricmp(const char *s1, const char *s2);
int strnicmp(const char *s1, const char *s2, size_t count);
char *strchr(const char *s, int ch);
char *strrchr(const char *s, int ch);
char *strstr(const char *str1, const char *str2);
char *stristr(const char *str1, const char *str2);
size_t strspn(const char *string, const char *control);
size_t strcspn(const char *string, const char *control);
char *strpbrk(const char *string, const char *control);
int strreplace(char *s1, const char s2, const char s3);

char *strlwr(char *s);
char *strupr(char *s);

