#include "Boats/FiskBoardingController.h"
#include "World/FiskPierWalk.h"
void AFiskBoardingController::ServerPierMode_Implementation(FGuid Profile,bool Enabled) { PierWalk->ServerMode(Profile,Enabled); }
void AFiskBoardingController::ServerPierInput_Implementation(FGuid Profile,int32 Generation,int32 Serial,FVector2D Axis,bool Sprint) { PierWalk->ServerInput(Profile,Generation,Serial,Axis,Sprint); }
void AFiskBoardingController::ClientPierMode_Implementation(bool Enabled,FVector Position,int32 Generation) { PierWalk->ReceiveMode(Enabled,Position,Generation); }
