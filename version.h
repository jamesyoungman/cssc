extern const char main_rcs_id[];
extern const char cssc_version[];

inline void
version() 
{
  fprintf(stderr, "%s\n%s\n", cssc_version, main_rcs_id);
}
