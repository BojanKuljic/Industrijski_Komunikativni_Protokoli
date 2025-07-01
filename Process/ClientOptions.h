#pragma once


bool RegisterService(int ServiceID, int ServiceType);

bool SendData(int ServiceID, char* data, int dataSize);

bool ReceiveData(char* data, int dataSize);

int RegistrationHandler(int ServiceID, int ServiceType);

bool SendDataHandler(int ServiceID);

bool CallbackHandler(int ServiceID);

bool ReplicateHandler(int ServiceID);

bool CloseHandler(int ServiceID);
