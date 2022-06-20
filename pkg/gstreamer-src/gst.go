// Package gst provides an easy API to create an appsink pipeline
package gst

/*
#cgo pkg-config: gstreamer-1.0 gstreamer-app-1.0

#include "gst.h"

*/
import "C"
import (
	"fmt"
	"time"
	"unsafe"
	"log"

	"github.com/pion/webrtc/v3"
	"github.com/pion/webrtc/v3/pkg/media"
)

func init() {
	go C.gstreamer_send_start_mainloop()
}

// Pipeline is a wrapper for a GStreamer Pipeline
type Pipeline struct {
	Pipeline   *C.GstElement
	videoTrack *webrtc.TrackLocalStaticSample
	audioTrack *webrtc.TrackLocalStaticSample
}

var pipeline *Pipeline

func CreateURIDecodeBinPipeline(uri string, videoTrack, audioTrack *webrtc.TrackLocalStaticSample) *Pipeline {
	pipelineStr := fmt.Sprintf("uridecodebin uri=%s name=dec ! x264enc ! video/x-h264,stream-format=byte-stream ! appsink name=appsinkvideo dec. ! queue ! audioconvert ! audioresample ! opusenc ! appsink name=appsinkaudio", uri)
	pipelineStrUnsafe := C.CString(pipelineStr)
	defer C.free(unsafe.Pointer(pipelineStrUnsafe))

	return &Pipeline{
		Pipeline:   C.gstreamer_send_create_pipeline(pipelineStrUnsafe),
		videoTrack: videoTrack,
		audioTrack: audioTrack,
	}
}

// Start starts the GStreamer Pipeline
func (p *Pipeline) Start() {
	C.gstreamer_send_start_pipeline(p.Pipeline)
}

// Stop stops the GStreamer Pipeline
func (p *Pipeline) Stop() {
	C.gstreamer_send_stop_pipeline(p.Pipeline)
}

//export goHandleAudioPipelineBuffer
func goHandleAudioPipelineBuffer(buffer unsafe.Pointer, bufferLen C.int, duration C.int) {
	log.Printf("goHandleAudioPipelineBuffer %v", pipeline.audioTrack)
	if err := pipeline.audioTrack.WriteSample(media.Sample{Data: C.GoBytes(buffer, bufferLen), Duration: time.Duration(duration)}); err != nil {
		panic(err)
	}
	C.free(buffer)
}

//export goHandleVideoPipelineBuffer
func goHandleVideoPipelineBuffer(buffer unsafe.Pointer, bufferLen C.int, duration C.int) {
	log.Printf("goHandleVideoPipelineBuffer %v", pipeline.videoTrack)
	if err := pipeline.videoTrack.WriteSample(media.Sample{Data: C.GoBytes(buffer, bufferLen), Duration: time.Duration(duration)}); err != nil {
		panic(err)
	}
	C.free(buffer)
}
