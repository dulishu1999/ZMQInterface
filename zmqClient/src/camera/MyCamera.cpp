#include "MyCamera.h"
#include <string.h>

CMyCamera::CMyCamera()
{
    m_hDevHandle        = NULL;
}

CMyCamera::~CMyCamera()
{
    if (m_hDevHandle)
    {
        MV_CC_DestroyHandle(m_hDevHandle);
        m_hDevHandle    = NULL;
    }
}


int CMyCamera::EnumDevices(MV_CC_DEVICE_INFO_LIST* pstDevList)
{
    // Enum GIGE Devices
    int nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, pstDevList);
    if (MV_OK != nRet)
    {
        return MY_FAIL;
    }

    return MY_OK;
}

// ���豸
int CMyCamera::Open(MV_CC_DEVICE_INFO* pstDeviceInfo)
{
    if (NULL == pstDeviceInfo)
    {
        return MY_FAIL;
    }

    int nRet  = MV_CC_CreateHandle(&m_hDevHandle, pstDeviceInfo);
    if (MV_OK != nRet)
    {
        return MY_FAIL;
    }

    nRet = MV_CC_OpenDevice(m_hDevHandle);
    if (MV_OK != nRet)
    {
        MV_CC_DestroyHandle(&m_hDevHandle);
        return MY_FAIL;
    }

    return MY_OK;
}


// �ر��豸
int CMyCamera::Close()
{
    if (NULL == m_hDevHandle)
    {
        return MY_FAIL;
    }
    MV_CC_CloseDevice(m_hDevHandle);
    return MV_CC_DestroyHandle(m_hDevHandle);
}


// ����ץͼ
int CMyCamera::StartGrabbing()
{
    if (NULL == m_hDevHandle)
    {
        return MY_FAIL;
    }

    return MV_CC_StartGrabbing(m_hDevHandle);
}


// ֹͣץͼ
int CMyCamera::StopGrabbing()
{
    if (NULL == m_hDevHandle)
    {
        return MY_FAIL;
    }

    return MV_CC_StopGrabbing(m_hDevHandle);
}

int CMyCamera::GetOneFrameTimeout(unsigned char* pData, unsigned int* pnDataLen, unsigned int nDataSize, MV_FRAME_OUT_INFO_EX* pFrameInfo, int nMsec)
{
    if (   NULL == m_hDevHandle 
        || NULL == pData 
        || NULL == pnDataLen 
        || NULL == pFrameInfo)
    {
        return MY_FAIL;
    }

    *pnDataLen  = 0;

    int nRet = MV_CC_GetOneFrameTimeout(m_hDevHandle, pData, nDataSize, pFrameInfo, nMsec);
    if (MV_OK != nRet)
    {
        return MY_FAIL;
    }

    *pnDataLen = pFrameInfo->nFrameLen;

    return MY_OK;
}

int CMyCamera::GetImageBuffer(IN void* handle, IN OUT MV_FRAME_OUT& stOutFrame, IN unsigned int  nMsec) {
    if (NULL == handle )
    {
        return MY_FAIL;
    }
    int nRet = MV_CC_GetImageBuffer(handle, &stOutFrame, nMsec);
    if (nRet == MV_OK)
    {
        printf("Get Image Buffer: Width[%d], Height[%d], FrameNum[%d]\n",stOutFrame.stFrameInfo.nWidth, stOutFrame.stFrameInfo.nHeight, stOutFrame.stFrameInfo.nFrameNum);
        nRet = MV_CC_FreeImageBuffer(handle, &stOutFrame);
        if (nRet != MV_OK)
        {
            printf("Free Image Buffer fail! nRet [0x%x]\n", nRet);
            return MY_FAIL;
        }
        return MY_OK;
    }
    else
    {
        return MY_FAIL;
    }
}
// ������ʾ���ھ��
int CMyCamera::Display(void* hWnd)
{
    if (NULL == m_hDevHandle)
    {
        return MY_FAIL;
    }

    return MV_CC_Display(m_hDevHandle, hWnd);
}


int CMyCamera::SaveImage(MV_SAVE_IMAGE_PARAM_EX* pstParam)
{
    if (NULL == pstParam)
    {
        return MY_FAIL;
    }

    return MV_CC_SaveImageEx(pstParam);
}

// ע��ͼ�����ݻص�
int CMyCamera::RegisterImageCallBack(void(__stdcall* cbOutput)(unsigned char * pData, 
                                                                MV_FRAME_OUT_INFO_EX* pFrameInfo, 
                                                                void* pUser),
                                         void* pUser)
{
    if (NULL == m_hDevHandle)
    {
        return MY_FAIL;
    }

    return MV_CC_RegisterImageCallBackEx(m_hDevHandle, cbOutput, pUser);
}


// ע����Ϣ�쳣�ص�
int CMyCamera::RegisterExceptionCallBack(void(__stdcall* cbException)(unsigned int nMsgType, 
                                                                            void* pUser),
                                             void* pUser)
{
    if (NULL == m_hDevHandle)
    {
        return MY_FAIL;
    }

    return MV_CC_RegisterExceptionCallBack(m_hDevHandle, cbException, pUser);
}


// ��ȡInt�Ͳ������� Width��Height����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
int CMyCamera::GetIntValue(IN const char* strKey, OUT unsigned int *pnValue)
{
    if (NULL == m_hDevHandle || NULL == strKey || NULL == pnValue)
    {
        return MY_FAIL;
    }

    MVCC_INTVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_INTVALUE));
    int nRet = MV_CC_GetIntValue(m_hDevHandle, strKey, &stParam);
    if (MV_OK != nRet)
    {
        return MY_FAIL;
    }

    *pnValue = stParam.nCurValue;

    return MY_OK;
}


// ����Int�Ͳ������� Width��Height����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
int CMyCamera::SetIntValue(IN const char* strKey, IN unsigned int nValue)
{
    if (NULL == m_hDevHandle || NULL == strKey)
    {
        return MY_FAIL;
    }

    return MV_CC_SetIntValue(m_hDevHandle, strKey, nValue);
}


// ��ȡFloat�Ͳ������� ExposureTime��Gain����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
int CMyCamera::GetFloatValue(IN const char* strKey, OUT float *pfValue)
{
    if (NULL == m_hDevHandle || NULL == strKey || NULL == pfValue)
    {
        return MY_FAIL;
    }

    MVCC_FLOATVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_FLOATVALUE));
    int nRet = MV_CC_GetFloatValue(m_hDevHandle, strKey, &stParam);
    if (MV_OK != nRet)
    {
        return MY_FAIL;
    }

    *pfValue = stParam.fCurValue;

    return MY_OK;
}


// ����Float�Ͳ������� ExposureTime��Gain����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
int CMyCamera::SetFloatValue(IN const char* strKey, IN float fValue)
{
    if (NULL == m_hDevHandle || NULL == strKey)
    {
        return MY_FAIL;
    }

    return MV_CC_SetFloatValue(m_hDevHandle, strKey, fValue);
}


// ��ȡEnum�Ͳ������� PixelFormat����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
int CMyCamera::GetEnumValue(IN const char* strKey, OUT unsigned int *pnValue)
{
    if (NULL == m_hDevHandle || NULL == strKey || NULL == pnValue)
    {
        return MY_FAIL;
    }

    MVCC_ENUMVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_ENUMVALUE));
    int nRet = MV_CC_GetEnumValue(m_hDevHandle, strKey, &stParam);
    if (MV_OK != nRet)
    {
        return MY_FAIL;
    }

    *pnValue = stParam.nCurValue;

    return MY_OK;
}


// ����Enum�Ͳ������� PixelFormat����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
int CMyCamera::SetEnumValue(IN const char* strKey, IN unsigned int nValue)
{
    if (NULL == m_hDevHandle || NULL == strKey)
    {
        return MY_FAIL;
    }

    return MV_CC_SetEnumValue(m_hDevHandle, strKey, nValue);
}


// ��ȡBool�Ͳ������� ReverseX����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
int CMyCamera::GetBoolValue(IN const char* strKey, OUT bool *pbValue)
{
    if (NULL == m_hDevHandle || NULL == strKey || NULL == pbValue)
    {
        return MY_FAIL;
    }

    int nRet = MV_CC_GetBoolValue(m_hDevHandle, strKey, pbValue);
    if (MV_OK != nRet)
    {
        return MY_FAIL;
    }

    return MY_OK;
}


// ����Bool�Ͳ������� ReverseX����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
int CMyCamera::SetBoolValue(IN const char* strKey, IN bool bValue)
{
    if (NULL == m_hDevHandle || NULL == strKey)
    {
        return MY_FAIL;
    }

    return MV_CC_SetBoolValue(m_hDevHandle, strKey, bValue);
}


// ��ȡString�Ͳ������� DeviceUserID����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�UserSetSave
int CMyCamera::GetStringValue(IN const char* strKey, IN OUT char* strValue, IN unsigned int nSize)
{
    if (NULL == m_hDevHandle || NULL == strKey || NULL == strValue)
    {
        return MY_FAIL;
    }

    MVCC_STRINGVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_STRINGVALUE));
    int nRet = MV_CC_GetStringValue(m_hDevHandle, strKey, &stParam);
    if (MV_OK != nRet)
    {
        return MY_FAIL;
    }

    strcpy_s(strValue, nSize, stParam.chCurValue);

    return MY_OK;
}


// ����String�Ͳ������� DeviceUserID����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�UserSetSave
int CMyCamera::SetStringValue(IN const char* strKey, IN const char* strValue)
{
    if (NULL == m_hDevHandle || NULL == strKey)
    {
        return MY_FAIL;
    }

    return MV_CC_SetStringValue(m_hDevHandle, strKey, strValue);
}


// ִ��һ��Command������� UserSetSave����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
int CMyCamera::CommandExecute(IN const char* strKey)
{
    if (NULL == m_hDevHandle || NULL == strKey)
    {
        return MY_FAIL;
    }

    return MV_CC_SetCommandValue(m_hDevHandle, strKey);
}
bool CMyCamera::PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
    if (NULL == pstMVDevInfo)
    {
        printf("The Pointer of pstMVDevInfo is NULL!\n");
        return false;
    }
    if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
    {
        int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
        int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
        int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
        int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

        // print current ip and user defined name
        printf("CurrentIp: %d.%d.%d.%d\n", nIp1, nIp2, nIp3, nIp4);
        printf("UserDefinedName: %s\n\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
    }
    else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
    {
        printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
        printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
        printf("Device Number: %d\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);
    }
    else
    {
        printf("Not support.\n");
    }

    return true;
}