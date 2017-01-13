#ifndef _C_BLUETOOTH
#define _C_BLUETOOTH

#ifdef CPP
extern "C"
{
#endif

void Bluetooth_State(void);
int Bluetooth_Init(void);
int Bluetooth_Avk_Next(void);
int Bluetooth_Avk_Previous(void);
int Bluetooth_Avk_Pause(void);
int Bluetooth_Disconnect(void);
int Bluetooth_Avk_Play(void);
int Bluetooth_Auto_Connect(void);
int Bluetooth_Set_Device_Connectable(int enable);
int Bluetooth_Set_Device_Discoverable(int enable);
int Bluetooth_Set_Name(const char *bt_name);
int Bluetooth_Off(void);
int Bluetooth_On(void);
int Bluetooth_isPlaying(void);
int Bluetooth_isConnected(void);
int Bluetooth_isOn(void);
//void Bluetooth_Set_WorkDir(void);
int Bluetooth_Reset_Avk_Status(void);

#ifdef CPP
}
#endif

#endif
