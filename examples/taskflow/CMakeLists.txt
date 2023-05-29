list(APPEND TF_EXAMPLES
  simple 
  attach_data
  async
  subflow_async
  dependent_async
  observer 
  subflow 
  fibonacci 
  condition
  multi_condition
  switch_case
  do_while_loop
  while_loop
  if_else
  nested_if_else
  priority
  visualization 
  parallel_for 
  parallel_sort
  reduce 
  inclusive_scan
  exclusive_scan
  pipeline
  scalable_pipeline
  scalable_pipeline_with_deferred_tokens
  text_pipeline
  taskflow_pipeline
  parallel_graph_pipeline
  parallel_data_pipeline
  pipeline_with_deferred_tokens
  run 
  corun
  runtime
  worker_interface
  composition
  limited_concurrency
  cancel
)

foreach(example IN LISTS TF_EXAMPLES)
  carbin_cc_binary(
          NAME example_${example}
          SOURCES ${example}.cc
          DEPS rigel::taskflow ${CARBIN_DEPS_LINK}
          COPTS ${USER_CXX_FLAGS}
  )
endforeach()
