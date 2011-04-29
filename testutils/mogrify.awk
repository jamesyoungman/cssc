#! /usr/bin/awk

# This program sanitises an SCCS file to remove almost all the
# tell-tale detail.

/^c / {
  printf("c \n");
  next;
}

# Some implementations omit the trailing space.
/^c/ {
  printf("c\n");
  next;
}

/^[^]/ {
  printf("%d\n",  NR);
  next;
}

{
  print;
}
