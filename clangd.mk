# TODO(Barach): This is broken on Windows. Also, consider global compile_commands.json

# This generates the Clangd compile_commands.json file.
$(CLANGD_FILE): $(SRC)
	rm -f $(CLANGD_FILE);
	printf "[\n"													>> $(CLANGD_FILE);
	for c in $(SRC); do \
		printf "\t{\n"												>> $(CLANGD_FILE); \
		printf "\t\t\"directory\": \"$(CURDIR)/\",\n"				>> $(CLANGD_FILE); \
		printf "\t\t\"command\": \"gcc "							>> $(CLANGD_FILE); \
		for i in $(CFLAGS) $(LIBFLAGS); do \
			printf "%s" "$$i "										>> $(CLANGD_FILE); \
		done; \
		printf "\",\n"												>> $(CLANGD_FILE); \
		printf "\t\t\"file\": \"$$c\"\n"							>> $(CLANGD_FILE); \
		printf "\t},\n"												>> $(CLANGD_FILE); \
	done;
	printf "]\n"													>> $(CLANGD_FILE);

clangd-file: $(CLANGD_FILE)