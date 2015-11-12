#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <syslog.h>
#include <pthread.h>
#include <errno.h>
#include <gst/gst.h>
#include <dbus/dbus.h>
#include <dlt/dlt.h>
#include "Cameradae.h" 


DLT_DECLARE_CONTEXT(Camera_Daemon);


void  *Gstreamer_Pipeline(void*);
void  *Command_Adapter(void*);
void  *FSM(void*);
void  *Send_Signal(void*);


GDBusNodeInfo *g_mIntrospectionData=NULL;
GDBusInterfaceVTable g_mIfaceVTable;
guint g_mNameRequestId;
guint g_mRegistrationId;

GQueue *DbustoCommand_queue = NULL;
GQueue *CommandtoFSM_queue = NULL;
GQueue *FSMtoGst_queue = NULL;
GQueue *SingaltoDbus_queue = NULL;

GMainLoop *p_loop = NULL;
void *retval;

int main(int argc, char *argv[]) {

  /* register application */
  DLT_REGISTER_APP("Camera_Daemon","system camera daemon");

  /* register all contexts */
  DLT_REGISTER_CONTEXT(Camera_Daemon,"CON1","Context 1 for Logging");

  /* Start the nignx server*/
  system("service nginx start");

  /* Start the janus webrtc gateway*/
  system("/opt/janus/bin/janus -F /opt/janus/etc/janus/");

  /*Create the queue to store the command*/
  DbustoCommand_queue = g_queue_new();
  CommandtoFSM_queue = g_queue_new();
  FSMtoGst_queue = g_queue_new();



  pthread_t gstPipeline,commandAdapter,fsm;
   
  
  gs_su_info.e_status = E_IVOS_SERVICE_SU_NULL;
  g_mNameRequestId = g_bus_own_name(G_BUS_TYPE_SYSTEM,
                                    CAMERADAE_SERVICE,
                                    G_BUS_NAME_OWNER_FLAGS_NONE,
                                    CAMERACORE_acquired_bus_cb,
                                    CAMERACORE_acquired_name_cb,
                                    CAMERACORE_lost_name_cb,
                                    NULL,
                                    NULL); //GDestroyNotify

 

  //Create the threads
  if( 0 != pthread_create(&gstPipeline, NULL, Gstreamer_Pipeline, NULL)){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("Failed to ctreate the thread : Gstreamer_Pipeline"));
    return -1;
  }
  if( 0 != pthread_create(&commandAdapter, NULL, Command_Adapter, NULL)){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("Failed to ctreate the thread : Command_Adapter"));
    return -1;
  }
  if( 0 != pthread_create(&fsm, NULL, FSM, NULL)){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("Failed to ctreate the thread : FSM"));
    return -1;
  }

  
  /*wait the end of the threads */
  pthread_join(gstPipeline, &retval);
  pthread_join(commandAdapter, &retval);
  pthread_join(fsm, &retval);


  p_loop = g_main_loop_new(NULL, TRUE);
  if(!p_loop) {
    DLT_LOG(OTACON1,DLT_LOG_ERROR,DLT_STRING("IVOS_CAMERACORE Failed to create GMainLoop"));
        return -2;
  }
  
  g_main_loop_run(p_loop);
  g_main_loop_unref(p_loop);

  g_queue_free (DbustoCommand_queue);
  g_queue_free (CommandtoFSM_queue);
  g_queue_free (FSMtoGst_queue);
  
  /* unregister your contexts */
  DLT_UNREGISTER_CONTEXT(Camera_Daemon);

  /* unregister your application */
  DLT_UNREGISTER_APP();
  return 0;  
}
