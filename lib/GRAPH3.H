
void Blit_Char(int cx, int yc, char c, int color, int trans_flag);
void Blit_String(int x, int y, int color, char *string, int trans_flag);
void Create_Cool_Palette(void);
void Delay(int clicks);
void Get_Palette_Register(int index, RGB_color_ptr color);
void H_Line(int x1, int x2, int y, unsigned int color);
void Plot_Pixel_Fast(int x, int y, unsigned char color);
void Set_Palette_Register(int index, RGB_color_ptr color);
void Set_Video_Mode(int mode);
