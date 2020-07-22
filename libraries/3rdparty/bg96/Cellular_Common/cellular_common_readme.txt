
Cellular_Common

Cellular_Runtime
	Mandatory
	Files :
		cellular_runtime_custom.c
		cellular_runtime_standard.c

Cmd
	Optionnal 
		USE_CMD_CONSOLE = 0
		USE_CELPERF = 0
	Files :
		celperf.c
		cmd.c
	defines :
		USE_CMD_CONSOLE   (plf_features.h : Default [1 : Activated])
		USE_CELPERF       (plf_features.h : Default [1 : Activated])

DataCache_Supplier
	Optionnal
		USE_DC_MEMS = 0
		USE_SIMU_MEMS = 0
		USE_DC_GENERIC = 0

	Files :
		dc_generic.c
		dc_mems.c        
	Defines :	
		USE_DC_MEMS     (plf_features.h : Default [1 : Activated])
		USE_SIMU_MEMS   (plf_features.h : Default [1 : Activated])
		USE_DC_GENERIC  (plf_features.h : Default [0 : not Activated])

Error_Handler
	Mandatory
	Files :
		error_handler.c
	Defines :
		USE_TRACE_ERROR_HANDLER  (plf_sw_config.h : Default [0 : not Activated])

Setup
	Optionnal : 
	Files :
		app_select.c
		feeprom_utils.c
		menu_utils.c
		setup.c
	Defines :
		USE_DEFAULT_SETUP  (plf_features.h : Default [0 : Use setup menu])

Stack_Analysis
	Optionnal
		STACK_ANALYSIS_TRACE = 0
	Files :
		stack_analysis.c
	Defines :
		STACK_ANALYSIS_TRACE      (plf_sw_config.h : Default [1])

Time_Date
	Mandatory / Optionnal (voir gestion en flag -- A FAIRE)
	Files :
		time_date.c

Trace_Interface
	Mandatory
	Files :
		trace_interface.c

