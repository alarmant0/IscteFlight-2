#ifndef __PROTO_EVAL_H__
#define __PROTO_EVAL_H__

void _student_checkExistsFifoServidor_C1 (char *); 
void _student_triggerSignals_C2 ();                
CheckIn _student_getDadosPedidoUtilizador_C3_C4 ();
void _student_writeRequest_C5 (CheckIn, char *);   
void _student_configureTimer_C6 (int);             
void _student_waitForEvents_C7 ();                 
void _student_trataSinalSIGUSR1_C8 (int);          
void _student_trataSinalSIGHUP_C9 (int);           
void _student_trataSinalSIGINT_C10 (int);          
void _student_trataSinalSIGALRM_C11 (int);         

void _student_checkExistsDB_S1 (char *);                              
void _student_createFifo_S2 (char *);                                 
void _student_triggerSignals_S3 ();                                   
CheckIn _student_readRequest_S4 (char *);                             
int  _student_createServidorDedicado_S5 ();                            
void _student_trataSinalSIGINT_S6 (int);                              
void _student_deleteFifoAndExit_S7 ();                                
void _student_trataSinalSIGCHLD_S8 (int);                             
void _student_triggerSignals_SD9 ();                                  
int  _student_searchClientDB_SD10 (CheckIn, char *, CheckIn *);        
void _student_checkinClientDB_SD11 (CheckIn *, char *, int, CheckIn); 
void _student_sendAckCheckIn_SD12 (int);                              
void _student_closeSessionDB_SD13 (CheckIn, char *, int);             
void _student_trataSinalSIGUSR2_SD14 (int);                           

#endif