################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
Dev/%.obj: ../Dev/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"C:/ti/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=none -me -Ooff --include_path="D:/Nexcom/Firmware/01_Project/02_Tester/TR28386_T/TR28386_T/TR28386_T_CM" --include_path="D:/Nexcom/Firmware/01_Project/02_Tester/TR28386_T/TR28386_T/TR28386_T_CM/Dev" --include_path="D:/Nexcom/Firmware/01_Project/02_Tester/TR28386_T/TR28386_T/TR28386_T_CM/CSU" --include_path="D:/Nexcom/Firmware/01_Project/02_Tester/TR28386_T/TR28386_T/TR28386_T_CM/SDK/include" --include_path="D:/Nexcom/Firmware/01_Project/02_Tester/TR28386_T/TR28386_T/TR28386_T_CM/SDK/driverlib_cm" --include_path="C:/ti/c2000/C2000Ware_26_00_00_00/driverlib/f2838x/driverlib_cm/" --include_path="C:/ti/ti-cgt-arm_20.2.7.LTS/include" --define=DEBUG --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="Dev/$(basename $(<F)).d_raw" --obj_directory="Dev" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


