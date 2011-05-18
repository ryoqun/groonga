NULL =

# (cd ../../doc; echo "po_files = \\"; find pot -type f -name '*.pot' | sort | sed -e 's,^pot/,\t,g' -e 's,pot$,po \\,'; echo -n "\t\$(NULL)")
po_files =					\
	characteristic.po			\
	command_version.po			\
	commands.po				\
	commands_not_implemented.po		\
	developer.po				\
	execfile.po				\
	expr.po					\
	functions.po				\
	grnslap.po				\
	grntest.po				\
	http.po					\
	index.po				\
	install.po				\
	limitations.po				\
	news.po					\
	process.po				\
	pseudo_column.po			\
	reference.po				\
	spec.po					\
	troubleshooting.po			\
	tutorial.po				\
	type.po

# (cd ../../doc; echo "mo_files = \\"; find pot -type f -name '*.pot' | sort | sed -e 's,^pot/,\t,g' -e 's,pot$,mo \\,'; echo -n "\t\$(NULL)")
mo_files = \
	characteristic.mo \
	command_version.mo \
	commands.mo \
	commands_not_implemented.mo \
	developer.mo \
	execfile.mo \
	expr.mo \
	functions.mo \
	grnslap.mo \
	grntest.mo \
	http.mo \
	index.mo \
	install.mo \
	limitations.mo \
	news.mo \
	process.mo \
	pseudo_column.mo \
	reference.mo \
	spec.mo \
	troubleshooting.mo \
	tutorial.mo \
	type.mo \
	$(NULL)

# (cd ../../doc; echo "mo_files_relative_from_locale_dir = \\"; find pot -type f -name '*.pot' | sort | sed -e 's,^pot/,\tLC_MESSAGES/,g' -e 's,pot$,mo \\,'; echo -n "\t\$(NULL)")
mo_files_relative_from_locale_dir = \
	LC_MESSAGES/characteristic.mo \
	LC_MESSAGES/command_version.mo \
	LC_MESSAGES/commands.mo \
	LC_MESSAGES/commands_not_implemented.mo \
	LC_MESSAGES/developer.mo \
	LC_MESSAGES/execfile.mo \
	LC_MESSAGES/expr.mo \
	LC_MESSAGES/functions.mo \
	LC_MESSAGES/grnslap.mo \
	LC_MESSAGES/grntest.mo \
	LC_MESSAGES/http.mo \
	LC_MESSAGES/index.mo \
	LC_MESSAGES/install.mo \
	LC_MESSAGES/limitations.mo \
	LC_MESSAGES/news.mo \
	LC_MESSAGES/process.mo \
	LC_MESSAGES/pseudo_column.mo \
	LC_MESSAGES/reference.mo \
	LC_MESSAGES/spec.mo \
	LC_MESSAGES/troubleshooting.mo \
	LC_MESSAGES/tutorial.mo \
	LC_MESSAGES/type.mo \
	$(NULL)