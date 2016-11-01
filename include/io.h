hid_t io_init (const char *filename);

void io_finalize (hid_t file_id);

herr_t save_state (state* s, hid_t file_id);
