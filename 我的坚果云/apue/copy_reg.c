static int
copy_reg (const char *src_path, const char *dst_path,
    const struct cp_options *x, mode_t dst_mode, int *new_dst,
    struct stat const *src_sb)
{
  char *buf;
  int buf_size;
  int dest_desc;
  int source_desc;
  struct stat sb;
  struct stat src_open_sb;
  char *cp;
  int *ip;
  int return_val = 0;
  off_t n_read_total = 0;
  int last_write_made_hole = 0;
  int make_holes = (x->sparse_mode == SPARSE_ALWAYS);

  source_desc = open (src_path, O_RDONLY);
  if (source_desc < 0)
    {
      error (0, errno, _("cannot open %s for reading"), quote (src_path));
      return -1;
    }

  if (fstat (source_desc, &src_open_sb))
    {
      error (0, errno, _("cannot fstat %s"), quote (src_path));
      return_val = -1;
      goto close_src_desc;
    }

  /* Compare the source dev/ino from the open file to the incoming,
     saved ones obtained via a previous call to stat.  */
  if (! SAME_INODE (*src_sb, src_open_sb))
    {
      error (0, 0,
       _("skipping file %s, as it was replaced while being copied"),
       quote (src_path));
      return_val = -1;
      goto close_src_desc;
    }

  /* These semantics are required for cp.
     The if-block will be taken in move_mode.  */
  if (*new_dst)
    {
      dest_desc = open (dst_path, O_WRONLY | O_CREAT, dst_mode);
    }
  else
    {
      dest_desc = open (dst_path, O_WRONLY | O_TRUNC, dst_mode);

      if (dest_desc < 0 && x->unlink_dest_after_failed_open)
  {
    if (unlink (dst_path))
      {
        error (0, errno, _("cannot remove %s"), quote (dst_path));
        return_val = -1;
        goto close_src_desc;
      }

    /* Tell caller that the destination file was unlinked.  */
    *new_dst = 1;

    /* Try the open again, but this time with different flags.  */
    dest_desc = open (dst_path, O_WRONLY | O_CREAT, dst_mode);
  }
    }

  if (dest_desc < 0)
    {
      error (0, errno, _("cannot create regular file %s"), quote (dst_path));
      return_val = -1;
      goto close_src_desc;
    }

  /* Determine the optimal buffer size.  */

  if (fstat (dest_desc, &sb))
    {
      error (0, errno, _("cannot fstat %s"), quote (dst_path));
      return_val = -1;
      goto close_src_and_dst_desc;
    }

  buf_size = ST_BLKSIZE (sb);

#if HAVE_STRUCT_STAT_ST_BLOCKS
  if (x->sparse_mode == SPARSE_AUTO && S_ISREG (sb.st_mode))
    {
      /* Use a heuristic to determine whether SRC_PATH contains any
   sparse blocks. */

      if (fstat (source_desc, &sb))
  {
    error (0, errno, _("cannot fstat %s"), quote (src_path));
    return_val = -1;
    goto close_src_and_dst_desc;
  }

      /* If the file has fewer blocks than would normally
   be needed for a file of its size, then
   at least one of the blocks in the file is a hole. */
      if (S_ISREG (sb.st_mode)
    && sb.st_size / ST_NBLOCKSIZE > ST_NBLOCKS (sb))
  make_holes = 1;
    }
#endif

  /* Make a buffer with space for a sentinel at the end.  */

  buf = (char *) alloca (buf_size + sizeof (int));

  for (;;)
    {
      ssize_t n_read = read (source_desc, buf, buf_size);
      if (n_read < 0)
  {
#ifdef EINTR
    if (errno == EINTR)
      continue;
#endif
    error (0, errno, _("reading %s"), quote (src_path));
    return_val = -1;
    goto close_src_and_dst_desc;
  }
      if (n_read == 0)
  break;

      n_read_total += n_read;

      ip = 0;
      if (make_holes)
  {
    buf[n_read] = 1;  /* Sentinel to stop loop.  */

    /* Find first nonzero *word*, or the word with the sentinel.  */

    ip = (int *) buf;
    while (*ip++ == 0)
      ;

    /* Find the first nonzero *byte*, or the sentinel.  */

    cp = (char *) (ip - 1);
    while (*cp++ == 0)
      ;

    /* If we found the sentinel, the whole input block was zero,
       and we can make a hole.  */

    if (cp > buf + n_read)
      {
        /* Make a hole.  */
        if (lseek (dest_desc, (off_t) n_read, SEEK_CUR) < 0L)
    {
      error (0, errno, _("cannot lseek %s"), quote (dst_path));
      return_val = -1;
      goto close_src_and_dst_desc;
    }
        last_write_made_hole = 1;
      }
    else
      /* Clear to indicate that a normal write is needed. */
      ip = 0;
  }
      if (ip == 0)
  {
    size_t n = n_read;
    if (full_write (dest_desc, buf, n) != n)
      {
        error (0, errno, _("writing %s"), quote (dst_path));
        return_val = -1;
        goto close_src_and_dst_desc;
      }
    last_write_made_hole = 0;
  }
    }

  /* If the file ends with a `hole', something needs to be written at
     the end.  Otherwise the kernel would truncate the file at the end
     of the last write operation.  */

  if (last_write_made_hole)
    {
#if HAVE_FTRUNCATE
      /* Write a null character and truncate it again.  */
      if (full_write (dest_desc, "", 1) != 1
    || ftruncate (dest_desc, n_read_total) < 0)
#else
      /* Seek backwards one character and write a null.  */
      if (lseek (dest_desc, (off_t) -1, SEEK_CUR) < 0L
    || full_write (dest_desc, "", 1) != 1)
#endif
  {
    error (0, errno, _("writing %s"), quote (dst_path));
    return_val = -1;
  }
    }

close_src_and_dst_desc:
  if (close (dest_desc) < 0)
    {
      error (0, errno, _("closing %s"), quote (dst_path));
      return_val = -1;
    }
close_src_desc:
  if (close (source_desc) < 0)
    {
      error (0, errno, _("closing %s"), quote (src_path));
      return_val = -1;
    }

  return return_val;
}