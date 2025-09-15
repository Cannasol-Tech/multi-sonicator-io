var sonicator__control_8h =
[
    [ "sonicator_state_control_t", "df/d70/structsonicator__state__control__t.html", "df/d70/structsonicator__state__control__t" ],
    [ "SONICATOR_COMM_TIMEOUT_MS", "de/d13/sonicator__control_8h.html#a3a8bcea46ab85058d6fd1ad4c3c32ad4", null ],
    [ "SONICATOR_FAULT_DEBOUNCE_MS", "de/d13/sonicator__control_8h.html#aa1c769c234868a66d06b393acfbf20cb", null ],
    [ "SONICATOR_MAX_AMPLITUDE_PERCENT", "de/d13/sonicator__control_8h.html#a9266e0128c993063b7b3aac98faf4e9b", null ],
    [ "SONICATOR_MIN_AMPLITUDE_PERCENT", "de/d13/sonicator__control_8h.html#abe06b5505cf15123b31cf8b029b59679", null ],
    [ "SONICATOR_RESET_PULSE_MS", "de/d13/sonicator__control_8h.html#a91c49141120b41b4039582f54a59d839", null ],
    [ "SONICATOR_START_DELAY_MS", "de/d13/sonicator__control_8h.html#a73ff3fa20f363956206243467723cfdb", null ],
    [ "SONICATOR_STOP_DELAY_MS", "de/d13/sonicator__control_8h.html#a0f1d094ebabfc54ecb3292163fbb97b7", null ],
    [ "SONICATOR_WATCHDOG_TIMEOUT_MS", "de/d13/sonicator__control_8h.html#aafedba2f7dd17703f2fee746b1096bcf", null ],
    [ "sonicator_command_t", "de/d13/sonicator__control_8h.html#a235d05e692f1aa44c0a7770ba5f87eb4", [
      [ "SONICATOR_CMD_NONE", "de/d13/sonicator__control_8h.html#a235d05e692f1aa44c0a7770ba5f87eb4a987db97b0e167389251f476d7c8c22d0", null ],
      [ "SONICATOR_CMD_START", "de/d13/sonicator__control_8h.html#a235d05e692f1aa44c0a7770ba5f87eb4a67e775d5c114ee423be652c23762b4e5", null ],
      [ "SONICATOR_CMD_STOP", "de/d13/sonicator__control_8h.html#a235d05e692f1aa44c0a7770ba5f87eb4a59cde013ea811598dfd6c4e8f3d3fc44", null ],
      [ "SONICATOR_CMD_RESET_OVERLOAD", "de/d13/sonicator__control_8h.html#a235d05e692f1aa44c0a7770ba5f87eb4ae468797997229d74ba359310b851b758", null ],
      [ "SONICATOR_CMD_EMERGENCY_STOP", "de/d13/sonicator__control_8h.html#a235d05e692f1aa44c0a7770ba5f87eb4a46c3d8fff15b619067de0d72192b4ae0", null ]
    ] ],
    [ "sonicator_fault_t", "de/d13/sonicator__control_8h.html#a361c37dbeedf89f2fe09845eab7f0a99", [
      [ "SONICATOR_FAULT_NONE", "de/d13/sonicator__control_8h.html#a361c37dbeedf89f2fe09845eab7f0a99a45f67c744f0a84ef961991bbbcee8cb7", null ],
      [ "SONICATOR_FAULT_OVERLOAD", "de/d13/sonicator__control_8h.html#a361c37dbeedf89f2fe09845eab7f0a99a73567def603bb6f8f77826552276d5ec", null ],
      [ "SONICATOR_FAULT_FREQ_UNLOCK", "de/d13/sonicator__control_8h.html#a361c37dbeedf89f2fe09845eab7f0a99a5de4021b7d7257f7229804ea8c159aa8", null ],
      [ "SONICATOR_FAULT_COMM_TIMEOUT", "de/d13/sonicator__control_8h.html#a361c37dbeedf89f2fe09845eab7f0a99a6b2e009dc023f729b34c1387dd3ec373", null ],
      [ "SONICATOR_FAULT_HARDWARE", "de/d13/sonicator__control_8h.html#a361c37dbeedf89f2fe09845eab7f0a99a46b1755f047e81751b29ae6787e7d594", null ],
      [ "SONICATOR_FAULT_WATCHDOG", "de/d13/sonicator__control_8h.html#a361c37dbeedf89f2fe09845eab7f0a99ab5b14d7067e124bcf3c19b309db532e1", null ]
    ] ],
    [ "sonicator_state_t", "de/d13/sonicator__control_8h.html#ad0aeab8703f3a1439806186cba759315", [
      [ "SONICATOR_STATE_IDLE", "de/d13/sonicator__control_8h.html#ad0aeab8703f3a1439806186cba759315a8aaaacd92fec828be10c02ef2696f0b7", null ],
      [ "SONICATOR_STATE_STARTING", "de/d13/sonicator__control_8h.html#ad0aeab8703f3a1439806186cba759315a34a68840cf8e7502193a12370a443e01", null ],
      [ "SONICATOR_STATE_RUNNING", "de/d13/sonicator__control_8h.html#ad0aeab8703f3a1439806186cba759315aefd4187011c854e789baa073995f0def", null ],
      [ "SONICATOR_STATE_STOPPING", "de/d13/sonicator__control_8h.html#ad0aeab8703f3a1439806186cba759315a68fc7fb7e15c844aa2c6cda43bcc8d09", null ],
      [ "SONICATOR_STATE_FAULT", "de/d13/sonicator__control_8h.html#ad0aeab8703f3a1439806186cba759315a7ef335199557a3658616b70275094a47", null ],
      [ "SONICATOR_STATE_UNKNOWN", "de/d13/sonicator__control_8h.html#ad0aeab8703f3a1439806186cba759315a02f13d1f0ad8e9fc52f93cf9c25ad0cf", null ]
    ] ],
    [ "sonicator_begin", "de/d13/sonicator__control_8h.html#ac88614577067020f420441ba689e7241", null ],
    [ "sonicator_emergency_stop", "de/d13/sonicator__control_8h.html#a626ecb2d4d66e32401478259cfe3595f", null ],
    [ "sonicator_force_state", "de/d13/sonicator__control_8h.html#a4519e53de39614ddb883a4c2db06ea19", null ],
    [ "sonicator_get_status", "de/d13/sonicator__control_8h.html#a1ecfcd93a4b2bd32c9cab9f359b831ba", null ],
    [ "sonicator_inject_fault", "de/d13/sonicator__control_8h.html#a79b77d91a6416dd0447f9c6c9741ec36", null ],
    [ "sonicator_is_safe", "de/d13/sonicator__control_8h.html#a2309ace8e76b3132b8b2f0e7d0df9956", null ],
    [ "sonicator_reset_overload", "de/d13/sonicator__control_8h.html#a21c76fda64cb8f4ddaf2f55f3567ec5e", null ],
    [ "sonicator_reset_statistics", "de/d13/sonicator__control_8h.html#a85b84eadaf4c625391673e214901144f", null ],
    [ "sonicator_set_amplitude", "de/d13/sonicator__control_8h.html#ad48d366d35d3f9fc640589b2b5623aa2", null ],
    [ "sonicator_set_simulation_mode", "de/d13/sonicator__control_8h.html#abe9ba511a647d9236e12897ce8cf7271", null ],
    [ "sonicator_start", "de/d13/sonicator__control_8h.html#aab49fb783e216647f53db17fbb6b62b1", null ],
    [ "sonicator_state_to_string", "de/d13/sonicator__control_8h.html#abe828539c1f97df3497b972c3c72c75f", null ],
    [ "sonicator_stop", "de/d13/sonicator__control_8h.html#a4c28efedf324a8f71669e203de81ca47", null ],
    [ "sonicator_update", "de/d13/sonicator__control_8h.html#adfd3a8b6450a1a09bf17d4865c32569c", null ]
];