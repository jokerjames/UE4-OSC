#include "OscPrivatePCH.h"
#include "OscFunctionLibrary.h"
#include "oscpack/osc/OscOutboundPacketStream.h"


UOscFunctionLibrary::UOscFunctionLibrary(const class FPostConstructInitializeProperties& PCIP)
  : Super(PCIP)
{
}

template <class T>
static inline
void PopValueImpl(const FOscDataStruct & input, FOscDataStruct & output, T & Value)
{
    output = input;
    Value = output.Pop().GetValue<T>();
}

void UOscFunctionLibrary::PopBool(const FOscDataStruct & input, FOscDataStruct & output, bool & Value)
{
    PopValueImpl(input, output, Value);
}

void UOscFunctionLibrary::PopFloat(const FOscDataStruct & input, FOscDataStruct & output, float & Value)
{
    PopValueImpl(input, output, Value);
}

void UOscFunctionLibrary::PopInt(const FOscDataStruct & input, FOscDataStruct & output, int32 & Value)
{
    PopValueImpl(input, output, Value);
}

void UOscFunctionLibrary::PopString(const FOscDataStruct & input, FOscDataStruct & output, FName & Value)
{
    PopValueImpl(input, output, Value);
}


void UOscFunctionLibrary::PushBool(FOscDataStruct input, bool Value, FOscDataStruct & output)
{
    output = input;
    FOscDataElemStruct elem;
    elem.SetBool(Value);
    output.Queue.Add(elem);
}

void UOscFunctionLibrary::PushFloat(FOscDataStruct input, float Value, FOscDataStruct & output)
{
    output = input;
    FOscDataElemStruct elem;
    elem.SetFloat(Value);
    output.Queue.Add(elem);
}

void UOscFunctionLibrary::PushInt(FOscDataStruct input, int32 Value, FOscDataStruct & output)
{
    output = input;
    FOscDataElemStruct elem;
    elem.SetInt(Value);
    output.Queue.Add(elem);
}

void UOscFunctionLibrary::PushString(FOscDataStruct input, FName Value, FOscDataStruct & output)
{
    if(Value.GetDisplayNameEntry()->IsWide())
    {
        const auto tmp = Value.GetPlainNameString();
        UE_LOG(LogOSC, Error, TEXT("Invalid string argument \"%s\": ASCII only"), *tmp);
        return;
    }

    output = input;
    FOscDataElemStruct elem;
    elem.SetString(Value);
    output.Queue.Add(elem);
}

void UOscFunctionLibrary::SendOsc(FName Address, const FOscDataStruct & Data, int32 TargetIndex)
{
    if(Address.GetDisplayNameEntry()->IsWide())
    {
        const auto tmp = Address.GetPlainNameString();
        UE_LOG(LogOSC, Error, TEXT("Invalid OSC address \"%s\": ASCII only"), *tmp);
        return;
    }
    
    static_assert(sizeof(uint8) == sizeof(char), "Cannot cast uint8 to char");
    uint8 buffer[1024];

    osc::OutboundPacketStream output((char *)buffer, sizeof(buffer));
    output << osc::BeginMessage(Address.GetPlainANSIString());
    for(int32 i=Data.Index, n=Data.Queue.Num(); i<n; ++i)
    {
        const auto & elem = Data.Queue[i];
        if(elem.IsFloat())
        {
            output << (float)elem.AsFloatValue();
        }
        else if(elem.IsInt())
        {
            output << (int32)elem.AsIntValue();
        }
        else if(elem.IsBool())
        {
            output << elem.AsBoolValue();
        }
        else if(elem.IsString())
        {
            output << elem.AsStringValue().GetPlainANSIString();
        }
    }
    output << osc::EndMessage;

    check(reinterpret_cast<const void *>(buffer) == reinterpret_cast<const void *>(output.Data()));
    GetMutableDefault<UOscSettings>()->Send(buffer, output.Size(), TargetIndex);
}
