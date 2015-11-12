#include "Cameradae.h"


extern GQueue *SingaltoDbus_queue ;
DLT_IMPORT_CONTEXT(Camera_Daemon) ;

void  *Send_Signal(void*){

  SignaltoDbus signal_t ;
  DLT_LOG(Camera_Daemon, DLT_LOG_INFO, DLT_STRING("Send_Signal start "));
  
  while(1){
    /*Get the signal from the Gst_pipeline*/
    while(g_queue_is_empty(SingaltoDbus_queue)){};

    signal_t = g_queue_pop_head(SingaltoDbus_queue);
  	/* 向dbus发送signal函数 */
  	DLT_LOG(Camera_Daemon, DLT_LOG_INFO, DLT_STRING("Send_Signal g_dbus_connection_emit_signal start"));
  	g_dbus_connection_emit_signal(g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL),
                  TIZEN_PREFIX,   /* the unique bus name for the destination for the signal or NULL to emit to all listeners */
                  CAMERADAE_OBJ_PATH,
                  CAMERADAE_SERVICE,
                  signal_t.signalname,
                  signal_t.parameters,
                  NULL);
  	DLT_LOG(Camera_Daemon, DLT_LOG_INFO, DLT_STRING("Send_Signal g_dbus_connection_emit_signal end"));
 
    DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" Signal has been send to Dbus."));
  }  
}
