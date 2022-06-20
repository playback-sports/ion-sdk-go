#include "gst.h"

#include <gst/app/gstappsrc.h>

typedef struct SampleHandlerUserData {
} SampleHandlerUserData;

GMainLoop *gstreamer_send_main_loop = NULL;
void gstreamer_send_start_mainloop(void) {
  gstreamer_send_main_loop = g_main_loop_new(NULL, FALSE);

  g_main_loop_run(gstreamer_send_main_loop);
}

static gboolean gstreamer_send_bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
  switch (GST_MESSAGE_TYPE(msg)) {

  case GST_MESSAGE_EOS:
    g_print("End of stream\n");
    exit(1);
    break;

  case GST_MESSAGE_ERROR: {
    gchar *debug;
    GError *error;

    gst_message_parse_error(msg, &error, &debug);
    g_free(debug);

    g_printerr("Error: %s\n", error->message);
    g_error_free(error);
    exit(1);
  }
  default:
    break;
  }

  return TRUE;
}

GstFlowReturn gstreamer_send_new_audio_sample_handler(GstElement *object, gpointer user_data) {
  GstSample *sample = NULL;
  GstBuffer *buffer = NULL;
  gpointer copy = NULL;
  gsize copy_size = 0;
  SampleHandlerUserData *s = (SampleHandlerUserData *)user_data;

  g_signal_emit_by_name (object, "pull-sample", &sample);
  if (sample) {
    buffer = gst_sample_get_buffer(sample);
    if (buffer) {
      gst_buffer_extract_dup(buffer, 0, gst_buffer_get_size(buffer), &copy, &copy_size);
      goHandleAudioPipelineBuffer(copy, copy_size, GST_BUFFER_DURATION(buffer));
    }
    gst_sample_unref (sample);
  }

  return GST_FLOW_OK;
}

GstFlowReturn gstreamer_send_new_video_sample_handler(GstElement *object, gpointer user_data) {
  GstSample *sample = NULL;
  GstBuffer *buffer = NULL;
  gpointer copy = NULL;
  gsize copy_size = 0;
  SampleHandlerUserData *s = (SampleHandlerUserData *)user_data;

  g_signal_emit_by_name (object, "pull-sample", &sample);
  if (sample) {
    buffer = gst_sample_get_buffer(sample);
    if (buffer) {
      gst_buffer_extract_dup(buffer, 0, gst_buffer_get_size(buffer), &copy, &copy_size);
      goHandleVideoPipelineBuffer(copy, copy_size, GST_BUFFER_DURATION(buffer));
    }
    gst_sample_unref (sample);
  }

  return GST_FLOW_OK;
}


GstElement *gstreamer_send_create_pipeline(char *pipeline) {
  gst_init(NULL, NULL);
  GError *error = NULL;
  return gst_parse_launch(pipeline, &error);
}

void gstreamer_send_start_pipeline(GstElement *pipeline) {
  SampleHandlerUserData *s = calloc(1, sizeof(SampleHandlerUserData));

  GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  gst_bus_add_watch(bus, gstreamer_send_bus_call, NULL);
  gst_object_unref(bus);

  GstElement *appsinkaudio = gst_bin_get_by_name(GST_BIN(pipeline), "appsinkaudio");
  g_object_set(appsinkaudio, "emit-signals", TRUE, NULL);
  g_signal_connect(appsinkaudio, "new-sample", G_CALLBACK(gstreamer_send_new_audio_sample_handler), s);
  gst_object_unref(appsinkaudio);

  GstElement *appsinkvideo = gst_bin_get_by_name(GST_BIN(pipeline), "appsinkvideo");
  g_object_set(appsinkvideo, "emit-signals", TRUE, NULL);
  g_signal_connect(appsinkvideo, "new-sample", G_CALLBACK(gstreamer_send_new_video_sample_handler), s);
  gst_object_unref(appsinkvideo);

  gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void gstreamer_send_stop_pipeline(GstElement *pipeline) {
  gst_element_set_state(pipeline, GST_STATE_NULL);
}


