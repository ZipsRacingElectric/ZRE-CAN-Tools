BIN_DIR := ../../bin

all: $(BIN_DIR)/bms $(BIN_DIR)/can-dbc $(BIN_DIR)/setup-charger $(BIN_DIR)/setup-vehicle

$(BIN_DIR)/bms: bms.sh
	mkdir -p $(BIN_DIR)
	cp $< $@
	chmod a+x $@

$(BIN_DIR)/can-dbc: can_dbc.sh
	mkdir -p $(BIN_DIR)
	cp $< $@
	chmod a+x $@

$(BIN_DIR)/setup-charger: setup_charger.sh
	mkdir -p $(BIN_DIR)
	cp $< $@
	chmod a+x $@

$(BIN_DIR)/setup-vehicle: setup_vehicle.sh
	mkdir -p $(BIN_DIR)
	cp $< $@
	chmod a+x $@