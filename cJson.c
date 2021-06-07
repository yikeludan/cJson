#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include "cJson.h"

/*int main(){
    return 0;
}*/

static void *(*cJSON_malloc)(size_t sz) = malloc;

static void (*cJSON_free)(void *ptr) = free;

static const char * ep;//出错位置的指针

cJSON *cJSON_New_Item(){
    cJSON * node = (cJSON *)cJSON_malloc(sizeof(cJSON));
    if(node){
        memset(node,0,sizeof(cJSON));
    }
    return node;
}
const char *skip(const char *in){
    while (in && *in && ((unsigned char)*in<=32))//ASNCLL前32位 都是无效字符
    {
        in++;
    }
    return in;
}
const char * parse_String(cJSON *item,const char *str){
    char *ptr = str+1;
    char *ptr2;
    char *out;
    int len = 0;
    if(*str !='\"'){//判断不是字符串
        ep = str;//
        return NULL;
    }
    while (*ptr !='\"'&&*ptr&&++len){
        if(*ptr++ == '\\'){
            ptr++;
        }
    }
    out = (char *)cJSON_malloc(len+1);
    if(!out){
        return NULL;
    }
    ptr = str+1;//指针重新偏移回来
    ptr2 = out;
    while (*ptr != '\"' && *ptr){
        if(*ptr!='\\'){
            *ptr2++ = *ptr++;  //字符串深拷贝
        }
    }
    *(ptr2+1) = NULL;//help GC
    *ptr2 = 0;
    if(*ptr == '\"'){
        ptr++;
    }
    item->valueString = out;
    item->type = CJSON_STR;
    return ptr;
}

const char * parse_Number(cJSON *item,const char *num){
    double n = 0;//最终数字
    int sign = 1;//正负号
    int signSubScale = 1;
    int scale = 0;
    int subscale = 0;
    if(*num == '-'){
        sign = -1;
        num++;
    }
    if(*num == '0'){
        num++;
    }
    if (*num >= '0' && *num <= '9'){
        do{
            //1920
            n = (n*10.0) + (*num++ - '0');
        }while (*num >= '0' && *num <= '9');
    }
    if (*num == '.'&& num[1] >= '0'
                   && num[1] <= '9')
    {
        //12345.6789
        num++;
        do
        {
            n = (n*10.0) + (*num++ - '0');
            scale--;
        } while (*num >= '0' && *num <= '9');
    }

    if (*num == 'e' || *num == 'E')
    {
        num++;
        if (*num == '+') num++;
        else if (*num == '-')
        {
            signSubScale = -1;
            num++;
        }
        do
        {
            subscale = (subscale*10.0) + (*num++ - '0');
        } while (*num >= '0' && *num <= '9');
    }

    n = sign * n * pow(10.0, (scale + signSubScale*subscale));
    item->valueDouble = n;
    item->valueInt = (int)n;
    item->type = CJSON_NUM;
    return num;


}

const char * parse_Array(cJSON *item,const char *value){
    cJSON *child;
    if(*value !='['){
        ep = value;
        return NULL;
    }
    item->type = CJSON_ARR;
    value = skip(value+1);//t跳过无效字符 包括回车与制表符
    if(*value == ']')return value+1;

    item->child = child = cJSON_New_Item();
    if(!item->child)return NULL;

    value = skip(parse_Value(child,skip(value)));
    if(!value) return NULL;

    while (*value == ','){
        cJSON *newItem = cJSON_New_Item();
        if(!newItem) return NULL;

        child->next = newItem;
        newItem->pre = child;
        child = newItem;

        value = skip(parse_Value(child, skip(value + 1)));
        if (!value) return NULL;

    }
    if (*value == ']') return value + 1;
    ep = value;
    return NULL;

}

const char * parse_Object(cJSON *item,const char *value){
    cJSON *child;
    if(*value !='{'){
        ep = value;
        return NULL;
    }
    item->type =CJSON_OBJ;
    value = skip(value+1);
    if(*value == '}')return value+1;

    item->child = child =cJSON_New_Item();
    if(!item->child)return NULL;

    value = skip(parse_String(child,skip(value)));
    if(!*value) return NULL;
    child->string = child->valueString;
    child->valueString = NULL;
    if(*value != ':'){
        ep = value;
        return NULL;
    }
    value = parse_Value(child,skip(value+1));
    if(!value) return NULL;
    while (*value == ',')
    {
        cJSON *new_item;
        if (!(new_item = cJSON_New_Item())) return NULL;

        child->next = new_item;
        new_item->pre = child;
        child = new_item;

        value = skip(parse_String(child, skip(value + 1)));
        if (!value) return NULL;

        child->string = child->valueString;
        child->valueString = NULL;
        if (*value != ':')
        {
            ep = value;
            return NULL;
        }
        value = skip(parse_Value(child, skip(value + 1)));
        if (!value) return NULL;
    }

    if (*value == '}') return value + 1;

    ep = value;
    return NULL;
}

const char *parse_Value(cJSON *item,const char *value){
    if(!value){
        return NULL;
    }
    if(!strncmp(value,"false",5)){
        item->type = CJSON_FALSE;
        return value+5;
    }
    if(!strncmp(value,"true",4)){
        item->type = CJSON_TRUE;
        item->valueInt = 1;
        return value+4;
    }
    if(!strncmp(value,"null",4)){
        item->type = CJSON_NULL;
        return value+4;
    }
    if (*value == '\"') { return parse_String(item, value); }
    if (*value == '-' || (*value>='0' && *value <= '9')) {
        return parse_Number(item, value); }
    if (*value == '[') { return parse_Array(item, value); }
    if (*value == '{') { return parse_Object(item, value); }
    ep = value;
    return NULL;
}


cJSON *cJSON_Parse(const char *value){
    return cJSON_Parse_WithOpts(value,0,0);
}




cJSON *cJSON_Parse_WithOpts(const char *value,
        const char **return_pares_end,
        int require_null_terminated){
     const char *end = NULL;
     cJSON *c = cJSON_New_Item();
     ep = NULL;
     if(!c){
         return NULL;
     }
    end = parse_Value(c, skip(value));
    if (!end)
    {
        cJSON_Del(c);
        return NULL;
    }

    if (require_null_terminated)
    {
        end = skip(end);
        if (*end)
        {
            cJSON_Del(c);
            ep = end;
            return NULL;
        }
    }

    if (return_pares_end)
    {
        *return_pares_end = end;
    }

    return c;
}

void cJSON_Del(cJSON *c){
    cJSON *next;
    while (c)
    {
        next = c->next;
        if (c->child) { cJSON_Del(c->child); }
        if (c->valueString) { cJSON_free(c->valueString); }
        if (c->string) { cJSON_free(c->string); }
        cJSON_free(c);
        c = next;
    }
}

char *print_value(cJSON *item, int depth, int fmt){
    char *out = NULL;
    if(!item) return NULL;
    switch ((item->type) & 255)
    {

        case 2: out = cJSON_strdup("null");  break;
        case 0: out = cJSON_strdup("false"); break;
        case 1: out = cJSON_strdup("true"); break;
        case 3: out = print_number(item); break;
        case 4: out = print_string(item,0); break;
        case 5: out = print_array(item, depth, fmt); break;
        case 6: out = print_object(item, depth, fmt); break;
    }

}
char *print_number(cJSON *item){
    char *str = NULL;
    double d = item->valueDouble;
    if (d == 0)
    {
        str =(char *)cJSON_malloc(5);//help tail
        str = (char *)cJSON_malloc(2);
        if (str) strcpy(str, "0");
    }

        //整数 valueDouble = 10.09 valueInt = 10.00   10.00 - 10.09 fabs绝对值
    else if ((fabs((double)(item->valueInt) - d) <= DBL_EPSILON) /*&& d <= INT_MAX && d >= INT_MIN*/)
    {
        //2的64次方+1
        str = (char *)cJSON_malloc(21);
        if (str) sprintf(str, "%d", item->valueInt);
    }
    else
    {
        str = (char *)cJSON_malloc(64);
        if (str)
        {
            //1234xxxx.0
            if ((fabs(floor(d) - d) <= DBL_EPSILON) && fabs(d) < 1.0e60) { sprintf(str, "%.0f", d); }
            else if (fabs(d) < 1.0e-6 || fabs(1.0e9)) { sprintf(str, "%e", d); }
            else { sprintf(str, "%f", d); }
        }
    }

    return str;
}
char *print_string(cJSON *item, int isName){
    char *str = NULL;
    if(isName){ str = item->string; }
    else{ str = item->valueString; }
    char *ptr, *ptr2, *out;
    unsigned char token;
    int flag = 0, len = 0;

    for (ptr = str; *ptr; ptr++)
    {
        flag |= ((*ptr > 0 && *ptr < 32) || (*ptr == '\"') || (*ptr == '\\')) ? 1 : 0;
    }

    if (!flag)
    {
        len = ptr - str; //ptr 已经偏移到最后 这时后减去str 就是指针长度
        out = (char *)cJSON_malloc(len + 2 + 1);//2 等于两个双引号
        if (!out) return NULL;

        ptr2 = out;
        *ptr2++ = '\"';
        strcpy(ptr2, str);
        //"abcde"\0
        ptr2[len] = '\"';
        ptr2[len + 1] = '\0';

        return out;
    }

    if (!str)
    {
        out = (char *)cJSON_malloc(3);
        if (!out) return NULL;
        strcpy(out, "\"\"");
        return out;
    }

    ptr = str;
    while ((token = *ptr) && ++len)
    {
        if (strchr("\"\\\b\f\n\r\t", token)) { len++; }
        else if (token < 32) { len += 5; ptr++; }
    }

    out = (char *)cJSON_malloc(len + 3);
    if (!out) return NULL;

    ptr2 = out;
    ptr = str;
    *ptr2++ = '\"';

    while (*ptr)
    {
        if ((unsigned char)*ptr > 31 && *ptr != '\"' && *ptr != '\\')
        {
            *ptr2++ = *ptr++;
        }
        else
        {
            *ptr2++ = '\\';
            switch (token = *ptr)
            {
                case '\\':*ptr2++ = '\\'; break;
                case '\"':*ptr2++ = '\"'; break;
                case '\b':*ptr2++ = '\b'; break;
                case '\f':*ptr2++ = '\f'; break;
                case '\n':*ptr2++ = '\n'; break;
                case '\r':*ptr2++ = '\r'; break;
                case '\t':*ptr2++ = '\t'; break;
                default:
                    sprintf(str, "u%04x", token);
                    ptr2++;
                    break;
            }
        }
    }

    *ptr2++ = '\"';
    *ptr2++ = '\0';

    return out;
}

//["OSCAR",123,XXX]
char *print_array(cJSON *item, int depth, int fmt){
    char **entries; //char *entries[];
    char *out = NULL, *ptr, *ret;
    int len = 5, templen = 0, isFail = 0, i = 0; //"[]"\0
    cJSON *child = item->child;
    int numEntries = 0;

    while (child)
    {
        //广度优先
        numEntries++;
        child = child->next;
    }

    if (!numEntries)
    {
        out = (char *)cJSON_malloc(3);
        if (out) strcpy(out, "[]");
        return out;
    }

    entries = (char **)cJSON_malloc(numEntries * sizeof(char *));
    if (!entries) return NULL;
    memset(entries, 0, numEntries * sizeof(char *));

    child = item->child;
    while (child)
    {
        ret = print_value(child, depth + 1, fmt);
        entries[i++] = ret;
        if (ret) { len += strlen(ret) + 2 + (fmt ? 1 : 0); }
        else { isFail = 1; }

        child = child->next;
    }

    if (!isFail) { out = (char *)cJSON_malloc(len); }
    if (!out) { isFail = 1; }

    if (isFail)
    {
        for (int i = 0; i < numEntries; i++)
        {
            if (entries[i])
            {
                cJSON_free(entries[i]);
            }
        }
        cJSON_free(entries);

        return NULL;
    }

    *out = '[';
    ptr = out + 1;
    *ptr = '\0';
    for (i = 0; i < numEntries; i++)
    {
        templen = strlen(entries[i]);
        memcpy(ptr, entries[i], templen);
        ptr += templen;

        if (i != numEntries - 1)
        {
            *ptr++ = ',';
            if (fmt) { *ptr++ = ' '; }
            *ptr = '\0';
        }

        cJSON_free(entries[i]);
    }
    cJSON_free(entries);

    *ptr++ = ']';
    *ptr++ = '\0';

    return out;
}
char *print_object(cJSON *item, int depth, int fmt){
    char **entries = NULL, **names = NULL;
    char *out = NULL, *ptr, *ret, *str;
    int len = 7, i = 0, templen = 0;

    cJSON *child = item->child;
    int numEntries = 0, isFail = 0;

    while (child)
    {
        numEntries++;
        child = child->next;
    }

    if (!numEntries)
    {
        out = (char *)cJSON_malloc(fmt ? depth + 4 : 3);
        if (!out) return NULL;

        ptr = out;
        *ptr++ = '{';
        if (fmt)
        {
            *ptr++ = '\n';
            for (i = 0; i < depth - 1; i++)
            {
                *ptr++ = '\t';
            }
        }

        *ptr++ = '}';
        *ptr++ = '\0';

        return out;

    }

    entries = (char **)cJSON_malloc(numEntries * sizeof(char *));
    if (!entries) return NULL;

    names = (char **)cJSON_malloc(numEntries * sizeof(char *));
    if (!names) { cJSON_free(entries); return NULL; }

    memset(entries, 0, numEntries * sizeof(char *));
    memset(names, 0, numEntries * sizeof(char *));

    child = item->child;
    depth++;
    if (fmt) { len += depth; }

    while (child)
    {
        names[i] = str = print_string(child, 1);
        entries[i++] = ret = print_value(child, depth, fmt);
        if (str && ret) { len += strlen(ret) + strlen(str) + 2 + (fmt ? 2 + depth : 0); }
        else { isFail = 1; }

        child = child->next;
    }

    if (!isFail) { out = (char *)cJSON_malloc(len); }
    if (!out) { isFail = 1; }

    if (isFail)
    {
        for (i = 0; i < numEntries; i++)
        {
            if (names[i]) { cJSON_free(names[i]); }
            if (entries[i]) { cJSON_free(entries[i]); }
        }
        cJSON_free(names);
        cJSON_free(entries);

        return NULL;
    }

    *out = '{';
    ptr = out + 1;
    if (fmt) { *ptr++ = '\n'; }
    *ptr = '\0';

    for (i = 0; i < numEntries; i++)
    {
        if (fmt)
        {
            for (int j = 0; j < depth; j++) { *ptr++ = '\t'; }
        }
        templen = strlen(names[i]);
        memcpy(ptr, names[i], templen);
        ptr += templen;
        *ptr++ = ':';
        if (fmt) { *ptr++ = '\t'; }

        strcpy(ptr, entries[i]);
        ptr += strlen(entries[i]);

        if (i != numEntries - 1) { *ptr++ = ','; }
        if (fmt) { *ptr++ = '\n'; }
        *ptr = '\0';

        cJSON_free(names[i]);
        cJSON_free(entries[i]);
    }
    cJSON_free(names);
    cJSON_free(entries);

    if (fmt)
    {
        for (i = 0; i < depth - 1; i++)
        {
            *ptr++ = '\t';
        }
    }

    *ptr++ = '}';
    *ptr++ = '\0';

    return out;
}

char *cJSON_strdup(const char *str)
{
    char *copy = NULL;
    size_t len;

    len = strlen(str);
    copy =(char *) cJSON_malloc(len + 1);
    if (!copy) return NULL;
    memcpy(copy, str, len + 1);

    return copy;
}
char *cJSON_Print(cJSON *item){
    return print_value(item, 0, 1);
}
