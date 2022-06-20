#ifndef GST_H
#define GST_H

#include <glib.h>
#include <gst/gst.h>
#include <stdint.h>
#include <stdlib.h>

extern void goHandleAudioPipelineBuffer(void *buffer, int bufferLen, int samples);
extern void goHandleVideoPipelineBuffer(void *buffer, int bufferLen, int samples);

GstElement *gstreamer_send_create_pipeline(char *pipeline);
void gstreamer_send_start_pipeline(GstElement *pipeline);
void gstreamer_send_stop_pipeline(GstElement *pipeline);
void gstreamer_send_start_mainloop(void);

#endif
