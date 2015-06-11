extern "C" {
	void init();
	void clear();
	void load_image(char* filename, void* image);
	void draw_image(char* filename, void* image);
}
