extern const char main_rcs_id[];
extern const char cssc_version[];
void show_config_info(void);
void show_copyright(void);

inline void
version() 
{
  fprintf(stderr, "%s\n%s\n", cssc_version, main_rcs_id);
  show_copyright();
  show_config_info();
}
