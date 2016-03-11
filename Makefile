.PHONY: clean All

All:
	@echo "----------Building project:[ LFSReCon - Debug ]----------"
	@"$(MAKE)" -f  "LFSReCon.mk"
clean:
	@echo "----------Cleaning project:[ LFSReCon - Debug ]----------"
	@"$(MAKE)" -f  "LFSReCon.mk" clean
