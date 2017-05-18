#include <string.h>
#include "li.h"

extern li_string_t li_string_make(const char *s)
{
    li_string_t str;
    str.bytes = strdup(s);
    return str;
}

extern li_string_t li_string_copy(li_string_t str)
{
    return li_string_make(str.bytes);
}

extern void li_string_free(li_string_t str)
{
    free(str.bytes);
}

extern char *li_string_bytes(li_string_t str)
{
    return str.bytes;
}

extern li_character_t li_string_ref(li_string_t str, int idx)
{
    li_character_t c;
    const char *s = str.bytes;
    while (idx >= 0) {
        s += li_chr_decode(&c, s);
        idx--;
    }
    return c;
}

extern size_t li_string_length(li_string_t str)
{
    return li_chr_count(str.bytes);
}

extern li_cmp_t li_string_cmp(li_string_t st1, li_string_t st2)
{
    int res = strcmp(st1.bytes, st2.bytes);
    if (res < 0)
        return LI_CMP_LT;
    if (res > 0)
        return LI_CMP_GT;
    return LI_CMP_EQ;
}

extern li_string_t li_string_append(li_string_t str1, li_string_t str2)
{
    int n1 = strlen(str1.bytes);
    int n2 = strlen(str2.bytes);
    char *s = li_allocate(NULL, n1+n2+1, sizeof(*s));
    int i;
    for (i = 0; i < n1; ++i)
        s[i] = str1.bytes[i];
    for (i = 0; i < n2; ++i)
        s[n1+i] = str2.bytes[i];
    s[n1+n2] = '\0';
    str1 = li_string_make(s);
    free(s);
    return str1;
}
