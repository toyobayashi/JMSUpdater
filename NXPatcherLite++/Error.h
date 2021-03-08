enum NXError {
	DRAG_DROP_BAD_NAME = 1001,
	DIALOG_BAD_NAME,
	BAD_CRC,
	BAD_HEADER,
};

std::string ErrorHandler(int errorNumber);
