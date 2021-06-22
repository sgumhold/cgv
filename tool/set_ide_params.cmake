
function(set_ide_params target_name arguments)
    set_target_properties(${target_name} PROPERTIES VS_DEBUGGER_COMMAND $<TARGET_FILE:cgv_viewer>)
    set_target_properties(${target_name} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "${arguments}")
    set_target_properties(${target_name} PROPERTIES XCODE_SCHEME_EXECUTABLE $<TARGET_FILE:cgv_viewer>)
    set_target_properties(${target_name} PROPERTIES XCODE_SCHEME_ARGUMENTS "${arguments}")
endfunction()
