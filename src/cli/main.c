#include "qvk/qvk.h"
#include "pipe/graph.h"
#include "pipe/graph-io.h"
#include "pipe/graph-print.h"
#include "pipe/graph-export.h"
#include "pipe/global.h"
#include "pipe/modules/api.h"
#include "core/log.h"

#include <stdlib.h>

int main(int argc, char *argv[])
{
  // init global things, log and pipeline:
  dt_log_init(s_log_cli|s_log_pipe);
  dt_log_init_arg(argc, argv);
  dt_pipe_global_init();
  threads_global_init();

  int dump_nodes = 0;
  int output_cnt = 0;
  int config_start = 0; // start of arguments which are interpreted as additional config lines
  dt_graph_export_t param = {0};
  for(int i=0;i<argc;i++)
  {
    if(!strcmp(argv[i], "-g") && i < argc-1)
      param.p_cfgfile = argv[++i];
    else if(!strcmp(argv[i], "--quality") && i < argc-1)
      param.output[output_cnt].quality = atof(argv[++i]);
    else if(!strcmp(argv[i], "--width") && i < argc-1)
      param.output[output_cnt].max_width = atof(argv[++i]);
    else if(!strcmp(argv[i], "--height") && i < argc-1)
      param.output[output_cnt].max_height = atof(argv[++i]);
    else if(!strcmp(argv[i], "--filename") && i < argc-1)
      param.output[output_cnt].p_filename = argv[++i];
    else if(!strcmp(argv[i], "--format") && i < argc-1)
      {i++; param.output[output_cnt].mod = dt_token(argv[i]);}
    else if(!strcmp(argv[i], "--dump-modules"))
      param.dump_modules = 1;
    else if(!strcmp(argv[i], "--dump-nodes"))
      dump_nodes = 1;
    else if(!strcmp(argv[i], "--output") && i < argc-1 && ++i)
      param.output[output_cnt++].inst = dt_token(argv[i]);
    else if(!strcmp(argv[i], "--config"))
    { config_start = i+1; break; }

    // TODO: parse more options: filename, format related things etc
  }
  param.output_cnt = MAX(1, output_cnt);

  if(qvk_init()) exit(1);

  if(!param.p_cfgfile)
  {
    fprintf(stderr, "usage: vkdt-cli -g <graph.cfg>\n"
    "    [-d verbosity]                set log verbosity (mem,perf,pipe,cli,err,all)\n"
    "    [--dump-modules|--dump-nodes] write graphvis dot files to stdout\n"
    "    [--quality <0-100>]           jpg output quality\n"
    "    [--width <x>]                 max output width\n"
    "    [--height <y>]                max output height\n"
    "    [--filename <f>]              output filename (without extension or frame number)\n"
    "    [--format <fm>]               output format (o-jpg, o-bc1, o-pfm, ..)\n"
    "    [--output <inst>]             name the instance of the output to write (can use multiple)\n"
    "                                  this resets output specific options: quality, width, height\n"
    "    [--config]                    everything after this will be interpreted as additional cfg lines\n"
        );
    qvk_cleanup();
    exit(1);
  }

  dt_graph_t graph;
  dt_graph_init(&graph);

  param.extra_param_cnt = config_start ? argc - config_start : 0;
  param.p_extra_param   = argv + config_start;

  VkResult res = dt_graph_export(&graph, &param);

  // nodes we can only print after run() has been called:
  if(dump_nodes)
    dt_graph_print_nodes(&graph);

  dt_graph_cleanup(&graph);
  threads_global_cleanup();
  qvk_cleanup();
  exit(res);
}
