// 
// Notice Regarding Standards.  AMD does not provide a license or sublicense to
// any Intellectual Property Rights relating to any standards, including but not
// limited to any audio and/or video codec technologies such as MPEG-2, MPEG-4;
// AVC/H.264; HEVC/H.265; AAC decode/FFMPEG; AAC encode/FFMPEG; VC-1; and MP3
// (collectively, the "Media Technologies"). For clarity, you will pay any
// royalties due for such third party technologies, which may include the Media
// Technologies that are owed as a result of AMD providing the Software to you.
// 
// MIT license 
// 
//
// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#pragma once

#include "public/include/core/Context.h"
#include "public/include/components/Component.h"
#include "public/common/PropertyStorageExImpl.h"
#include "public/include/components/VideoCapture.h"
#include "public/src/components/VideoCapture/MFSource.h"

namespace amf
{
    //-------------------------------------------------------------------------------------------------

    class AMFVideoCaptureImpl :
        public AMFInterfaceBase,
        public AMFPropertyStorageExImpl<AMFComponentEx>
    {


    //-------------------------------------------------------------------------------------------------
    typedef AMFInterfaceImpl < AMFPropertyStorageExImpl <AMFOutput> > baseclassOutputProperty;

    class AMFOutputVideoCaptureImpl :
        public baseclassOutputProperty
    {
        friend class AMFVideoCaptureImpl;

    public:
        AMF_BEGIN_INTERFACE_MAP
            AMF_INTERFACE_ENTRY(AMFOutput)
            AMF_INTERFACE_CHAIN_ENTRY(AMFPropertyStorageExImpl <AMFOutput>)
            AMF_INTERFACE_CHAIN_ENTRY(baseclassOutputProperty)
        AMF_END_INTERFACE_MAP

        AMFOutputVideoCaptureImpl(AMFVideoCaptureImpl* pHost);
        virtual ~AMFOutputVideoCaptureImpl();

        // AMFOutput interface
        virtual AMF_RESULT AMF_STD_CALL  QueryOutput(AMFData** ppData);
    protected:
        AMF_RESULT              SubmitFrame(AMFData* pData);

        amf_int32                m_iQueueSize;  //queue size
        AMFVideoCaptureImpl*     m_pHost;       //host
        amf_int64                m_frameCount;  //frame count
        AMFQueue<AMFDataPtr>     m_VideoDataQueue; //video frame queue
    };
    typedef AMFInterfacePtr_T<AMFOutputVideoCaptureImpl>    AMFOutputVideoCaptureImplPtr;
    //-------------------------------------------------------------------------------------------------
    protected:
        class VideoCapturePollingThread : public amf::AMFThread
        {
        protected:
            AMFVideoCaptureImpl*         m_pHost;   //host
        public:
            VideoCapturePollingThread(AMFVideoCaptureImpl* pHost);
            ~VideoCapturePollingThread();
            virtual void Run();
        };

    public:
        // interface access
        AMF_BEGIN_INTERFACE_MAP
            AMF_INTERFACE_MULTI_ENTRY(AMFComponent)
            AMF_INTERFACE_MULTI_ENTRY(AMFComponentEx)
            AMF_INTERFACE_CHAIN_ENTRY(AMFPropertyStorageExImpl<AMFComponentEx>)
        AMF_END_INTERFACE_MAP


        AMFVideoCaptureImpl(AMFContext* pContext);
        virtual ~AMFVideoCaptureImpl();

        // AMFComponent interface
        virtual AMF_RESULT  AMF_STD_CALL  Init(AMF_SURFACE_FORMAT format, amf_int32 width, amf_int32 height);
        virtual AMF_RESULT  AMF_STD_CALL  ReInit(amf_int32 width, amf_int32 height);
        virtual AMF_RESULT  AMF_STD_CALL  Terminate();
        virtual AMF_RESULT  AMF_STD_CALL  Drain();
        virtual AMF_RESULT  AMF_STD_CALL  Flush();

        virtual AMF_RESULT  AMF_STD_CALL  SubmitInput(AMFData* /* pData */)                             { return AMF_NOT_SUPPORTED; };
        virtual AMF_RESULT  AMF_STD_CALL  QueryOutput(AMFData** /* ppData */)                           { return AMF_NOT_SUPPORTED; };
        virtual AMF_RESULT  AMF_STD_CALL  SetOutputDataAllocatorCB(AMFDataAllocatorCB* /* pCallback */) { return AMF_OK; };
        virtual AMF_RESULT  AMF_STD_CALL  GetCaps(AMFCaps** /* ppCaps */)                               { return AMF_NOT_SUPPORTED; };
        virtual AMF_RESULT  AMF_STD_CALL  Optimize(AMFComponentOptimizationCallback* /* pCallback */ )  { return AMF_OK; };
        virtual AMFContext* AMF_STD_CALL  GetContext()                                                  { return m_pContext; };

        // AMFComponentEx interface
        virtual amf_int32   AMF_STD_CALL  GetInputCount()                                               {  return 0;  };
        virtual amf_int32   AMF_STD_CALL  GetOutputCount()                                              {  return 1;  };

        virtual AMF_RESULT  AMF_STD_CALL  GetInput(amf_int32 /* index */, AMFInput** /* ppInput */)     { return AMF_NOT_SUPPORTED; };
        virtual AMF_RESULT  AMF_STD_CALL  GetOutput(amf_int32 index, AMFOutput** ppOutput);

        // AMFPropertyStorageObserver interface
        virtual void        AMF_STD_CALL  OnPropertyChanged(const wchar_t* pName);

    protected:
        AMF_RESULT PollStream();
        int SaveToFile(char* pData, int len);
        std::ofstream m_fileVideo;
        amf_int32 m_lenData;
        int WaveHeader(const WAVEFORMATEX *pWaveFormat, amf_int32 lenData, BYTE** ppData, amf_int32& sizeHeader);
        int AmbisonicFormatConvert(const void* pSrc, void* pDst, amf_int32 numSamples, const WAVEFORMATEX *pWaveFormat);

    protected:
        static const AMFSize   VideoFrameSizeList[];
        static const amf_int32 VideoFrameRateList[];

    private:
        AMFContextPtr               m_pContext;              //context
        mutable AMFCriticalSection  m_sync;                   //sync
        AMFOutputVideoCaptureImplPtr m_OutputStream;          //output object
        VideoCapturePollingThread    m_videoPollingThread;    //thread to pull frames from camera
        AMFMFSourceImpl              m_videoSource;           //MF source object
        bool                         m_bTerminated;           //flag to mark the termination
        amf_int64                    m_frameCount;            //number of frames handled

        AMFVideoCaptureImpl(const AMFVideoCaptureImpl&);
        AMFVideoCaptureImpl& operator=(const AMFVideoCaptureImpl&);
    };
    typedef AMFInterfacePtr_T<AMFVideoCaptureImpl>    AMFVideoCaptureImplPtr;
}