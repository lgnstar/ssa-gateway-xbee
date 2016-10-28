#define	PORT_TCP	9500

//tipo dispositivo
#define TIPO_MODBUS     1
#define TIPO_TCP        2
#define TIPO_XBEE       3


//tipo variavel
#define VAR_TEMP        1
#define VAR_HUMID       2
#define VAR_INTERRUP    3
#define VAR_RELE        4

//protocolo tcp:
//funcao tcp:
#define FUNC_PEDVAR     1
#define FUNC_RESPVAR    2
#define FUNC_INI        3
#define FUNC_INIOK	4
#define FUNC_INIID	5

//pedido de variavel:
//[func][var]
//resposta variavel:
//[func][var][val1][val2][val3][val4]

//inicio microcontrolador
//[func][ip1][ip2][ip3][ip4][mac1][mac2][mac3][mac4][mac5][mac6]

