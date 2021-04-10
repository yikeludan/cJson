#pragma once

#ifndef UNTITLED10_CJSON_H
#define UNTITLED10_CJSON_H


#define CJSON_FALSE       0;
#define CJSON_TRUE        1;
#define CJSON_NULL        2;
#define CJSON_NUM         3;
#define CJSON_STR         4;
#define CJSON_ARR         5;
#define CJSON_OBJ         6;
/*
功能：创建一个string值为name的cJSON_False节点，并添加到object
*/
#define cJSON_AddFalseToObject(object, name) \
	cJSON_AddItemToObject(object, name, cJSON_CreateFalse())


/*创建一个string值为name的cJSON_True节点,并添加到object节点*/
#define cJSON_AddTrueToObject(object,name) \
	cJSON_AddItemToObject(object, name, cJSON_CreateTrue())

/*创建一个string值为name的cJSON_Bool/False节点,并添加到object节点*/
#define cJSON_AddBoolToObject(object,name, b) \
	cJSON_AddItemToObject(object, name, cJSON_CreateBool(b))

/*创建一个string值为name的cJSON_NULL节点,并添加到object节点*/
#define cJSON_AddNULLToObject(object,name) \
	cJSON_AddItemToObject(object, name, cJSON_CreateNull())

/*创建一个string值为name的cJSON_Number节点,并添加到object节点*/
#define cJSON_AddNumberToObject(object,name, number) \
	cJSON_AddItemToObject(object, name, cJSON_CreateNumber(number))

/*创建一个string值为name的cJSON_String节点,并添加到object节点*/
#define cJSON_AddStringToObject(object,name, s) \
	cJSON_AddItemToObject(object, name, cJSON_CreateString(s))

#define cJSON_SetIntValue(object, val)\
	((object)?((object)->valueInt=(object)->valueDouble=(val)):(val))

#define cJSON_SetNumberValue(object, val)\
	((object)?((object)->valueInt=(object)->valueDouble=(val)):(val))

typedef struct cJSON{
    struct cJSON *next,*pre;
    struct cJSON *child;
    int type;//array obj类型需要设立节点
    char *valueString;
    int valueInt;
    double valueDouble;
    char *string;
}cJSON;


 /**
  * 常用解析函数
  * @param value char 指针 指向带解析的json数据
  * @return json树的根节点
  */
cJSON *cJSON_Parse(const char *value);

cJSON *cJSON_New_Item();
/**
 * 跳过无意义的制表符 和回车字符
 * @param in
 * @return
 */
const char *skip(const char *in);

const char *parse_Value(cJSON *item,const char *value);

const char * parse_String(cJSON *item,const char *str);

const char * parse_Number(cJSON *item,const char *num);

const char * parse_Array(cJSON *item,const char *value);

const char * parse_Object(cJSON *item,const char *value);
cJSON *cJSON_Parse_WithOpts(const char *value,
                            const char **return_pares_end,
                            int require_null_terminated);

/**
 * 从item开始递归遍历 把json 结构体转换成char指针字符串
 * @param item
 * @return
 */
char *cJSON_Print(cJSON *item);

char *print_value(cJSON *item, int depth, int fmt);

char *cJSON_strdup(const char *str);

char *print_number(cJSON *item);
char *print_string(cJSON *item, int isName);

//["OSCAR",123,XXX]
char *print_array(cJSON *item, int depth, int fmt);
char *print_object(cJSON *item, int depth, int fmt);
/**
 * 删除节点 释放内存
 * @param c
 */
void cJSON_Del(cJSON *c);

/**
 * 将item 添加到obj子节点的双向链表的尾部
 * @param Obj
 * @param string
 * @param Item
 */
void cJSON_AddItemObj(cJSON *Obj,char *string,cJSON *Item);

/**
 * 将item 添加到Array子节点的双向链表的尾部
 * @param Array
 * @param Item
 */
void cJSON_AddItemArry(cJSON *Array,cJSON *Item);

/**
 * 创建一个对象节点
 * @return
 */
cJSON *cJson_CreateObj(void);

cJSON *cJson_CreateStr(const char *str);

cJSON *cJson_CreateNum(const double num);

cJSON *cJson_CreateArray(void);

cJSON *cJson_CreateBool(int flag);

cJSON *cJson_CreateTrue(void);

cJSON *cJson_CreateFalse(void);

cJSON *cJson_CreateNull(void);


#endif //UNTITLED10_CJSON_H
