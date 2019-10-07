/* NB IOT server parameters */
#define NB_IOT_SERVER_ADDRESS   "1.1.1.1"
#define NB_IOT_SERVER_PORT       1111

#ifdef NB_IOT_PROVIDER_Orange
   #define NB_IOT_PLMN           "20610"
   #define NB_IOT_APN            "iot.orange.be"
   #define NB_IOT_BAND           "AT+NBAND=20\r\n"
   #define NB_IOT_AUTOCONNECT    "TRUE"
#endif
#ifdef NB_IOT_PROVIDER_Proximus
   #define NB_IOT_PLMN           "20601"
   #define NB_IOT_APN            "m2minternet.proximus.be"
   #define NB_IOT_BAND           "AT+NBAND=20\r\n"
   #define NB_IOT_AUTOCONNECT    "FALSE"
#endif
#ifdef NB_IOT_PROVIDER_Telenet
   #define NB_IOT_PLMN           "20620"
   #define NB_IOT_APN            "iot.telenet.be"
   #define NB_IOT_BAND           "AT+NBAND=8\r\n"
   #define NB_IOT_AUTOCONNECT    "TRUE"
#endif