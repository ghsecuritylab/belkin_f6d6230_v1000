#############################################################
#
# libglib2
#
#############################################################
LIBGLIB2_VERSION_MAJOR = 2.20
LIBGLIB2_VERSION_MINOR = 1
LIBGLIB2_VERSION = $(LIBGLIB2_VERSION_MAJOR).$(LIBGLIB2_VERSION_MINOR)
LIBGLIB2_SOURCE = glib-$(LIBGLIB2_VERSION).tar.bz2
LIBGLIB2_SITE = http://ftp.gtk.org/pub/glib/$(LIBGLIB2_VERSION_MAJOR)

LIBGLIB2_AUTORECONF = NO
LIBGLIB2_LIBTOOL_PATCH = NO
LIBGLIB2_INSTALL_STAGING = YES
LIBGLIB2_INSTALL_TARGET = YES
LIBGLIB2_INSTALL_STAGING_OPT = DESTDIR=$(STAGING_DIR) LDFLAGS=-L$(STAGING_DIR)/usr/lib install

LIBGLIB2_CONF_ENV =	\
		ac_cv_func_posix_getpwuid_r=yes glib_cv_stack_grows=no \
		glib_cv_uscore=no ac_cv_func_strtod=yes \
		ac_fsusage_space=yes fu_cv_sys_stat_statfs2_bsize=yes \
		ac_cv_func_closedir_void=no ac_cv_func_getloadavg=no \
		ac_cv_lib_util_getloadavg=no ac_cv_lib_getloadavg_getloadavg=no \
		ac_cv_func_getgroups=yes ac_cv_func_getgroups_works=yes \
		ac_cv_func_chown_works=yes ac_cv_have_decl_euidaccess=no \
		ac_cv_func_euidaccess=no ac_cv_have_decl_strnlen=yes \
		ac_cv_func_strnlen_working=yes ac_cv_func_lstat_dereferences_slashed_symlink=yes \
		ac_cv_func_lstat_empty_string_bug=no ac_cv_func_stat_empty_string_bug=no \
		vb_cv_func_rename_trailing_slash_bug=no ac_cv_have_decl_nanosleep=yes \
		jm_cv_func_nanosleep_works=yes gl_cv_func_working_utimes=yes \
		ac_cv_func_utime_null=yes ac_cv_have_decl_strerror_r=yes \
		ac_cv_func_strerror_r_char_p=no jm_cv_func_svid_putenv=yes \
		ac_cv_func_getcwd_null=yes ac_cv_func_getdelim=yes \
		ac_cv_func_mkstemp=yes utils_cv_func_mkstemp_limitations=no \
		utils_cv_func_mkdir_trailing_slash_bug=no \
		ac_cv_have_decl_malloc=yes gl_cv_func_malloc_0_nonnull=yes \
		ac_cv_func_malloc_0_nonnull=yes ac_cv_func_calloc_0_nonnull=yes \
		ac_cv_func_realloc_0_nonnull=yes jm_cv_func_gettimeofday_clobber=no \
		gl_cv_func_working_readdir=yes jm_ac_cv_func_link_follows_symlink=no \
		utils_cv_localtime_cache=no ac_cv_struct_st_mtim_nsec=no \
		gl_cv_func_tzset_clobber=no gl_cv_func_getcwd_null=yes \
		gl_cv_func_getcwd_path_max=yes ac_cv_func_fnmatch_gnu=yes \
		am_getline_needs_run_time_check=no am_cv_func_working_getline=yes \
		gl_cv_func_mkdir_trailing_slash_bug=no gl_cv_func_mkstemp_limitations=no \
		ac_cv_func_working_mktime=yes jm_cv_func_working_re_compile_pattern=yes \
		ac_use_included_regex=no gl_cv_c_restrict=no \
		ac_cv_path_GLIB_GENMARSHAL=$(HOST_DIR)/usr/bin/glib-genmarshal ac_cv_prog_F77=no \
		ac_cv_func_posix_getgrgid_r=no \
		gt_cv_c_wchar_t=$(if $(BR2_USE_WCHAR),yes,no)

LIBGLIB2_CONF_OPT = --enable-shared \
		--enable-static

LIBGLIB2_DEPENDENCIES = uclibc gettext libintl host-pkgconfig host-libglib2

ifneq ($(BR2_ENABLE_LOCALE),y)
LIBGLIB2_DEPENDENCIES+=libiconv
endif

ifeq ($(BR2_PACKAGE_LIBICONV),y)
LIBGLIB2_CONF_OPT += --with-libiconv=gnu
LIBGLIB2_DEPENDENCIES+=libiconv
endif

$(eval $(call AUTOTARGETS,package,libglib2))

# libglib2 for the host
LIBGLIB2_HOST_DIR:=$(BUILD_DIR)/libglib2-$(LIBGLIB2_VERSION)-host
LIBGLIB2_HOST_BINARY:=$(HOST_DIR)/usr/bin/glib-genmarshal

$(DL_DIR)/$(LIBGLIB2_SOURCE):
	$(call DOWNLOAD,$(LIBGLIB2_SITE),$(LIBGLIB2_SOURCE))

$(STAMP_DIR)/host_libglib2_unpacked: $(DL_DIR)/$(LIBGLIB2_SOURCE)
	mkdir -p $(LIBGLIB2_HOST_DIR)
	$(INFLATE$(suffix $(LIBGLIB2_SOURCE))) $< | \
		$(TAR) $(TAR_STRIP_COMPONENTS)=1 -C $(LIBGLIB2_HOST_DIR) $(TAR_OPTIONS) -
	touch $@

$(STAMP_DIR)/host_libglib2_configured: $(STAMP_DIR)/host_libglib2_unpacked $(STAMP_DIR)/host_pkgconfig_installed
	(cd $(LIBGLIB2_HOST_DIR); rm -rf config.cache; \
		$(HOST_CONFIGURE_OPTS) \
		CFLAGS="$(HOST_CFLAGS)" \
		LDFLAGS="$(HOST_LDFLAGS)" \
		./configure \
		--prefix="$(HOST_DIR)/usr" \
		--sysconfdir="$(HOST_DIR)/etc" \
		--enable-shared \
		--disable-static \
		--disable-gtk-doc \
		--enable-debug=no \
	)
	touch $@

$(STAMP_DIR)/host_libglib2_compiled: $(STAMP_DIR)/host_libglib2_configured
	$(MAKE) -C $(LIBGLIB2_HOST_DIR)
	touch $@

$(STAMP_DIR)/host_libglib2_installed: $(STAMP_DIR)/host_libglib2_compiled
	$(HOST_MAKE_ENV) $(MAKE) -C $(LIBGLIB2_HOST_DIR) install
	touch $@

host-libglib2: $(STAMP_DIR)/host_libglib2_installed

host-libglib2-source: libglib2-source

host-libglib2-clean:
	rm -f $(addprefix $(STAMP_DIR)/host_libglib2_,unpacked configured compiled installed)
	-$(MAKE) -C $(LIBGLIB2_HOST_DIR) uninstall
	-$(MAKE) -C $(LIBGLIB2_HOST_DIR) clean

host-libglib2-dirclean:
	rm -rf $(LIBGLIB2_HOST_DIR)
