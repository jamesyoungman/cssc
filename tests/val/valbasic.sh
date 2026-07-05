#! /bin/sh

# valbasic.sh:  Basic tests for the "val" command.

# Import common functions & definitions.
. ../common/test-common.sh

g=f
s="s.${g}"

cleanup() {
    remove "${g}" "${s}"
}



docommand v1 "${admin} -n ${s}" 0 IGNORE IGNORE
docommand v2 "${vg_val} ${s}" 0 IGNORE IGNORE

docommand v3 "${vg_val} -r1.1 ${s}" 0 IGNORE IGNORE
docommand v4 "${vg_val} -s ${s}" 0 IGNORE IGNORE

# Having no args is an error.
docommand v5 "${vg_val}" 128 IGNORE IGNORE


# Module flag mismatch
docommand v6 "${vg_val} -mZ ${s}" 1 IGNORE IGNORE

# Change the module flag
docommand v7 "${admin} -fmZ ${s}" 0 IGNORE IGNORE

# Module flag match
docommand v8 "${vg_val} -mZ ${s}" 0 IGNORE IGNORE


# Type flag mismatch
docommand v9 "${vg_val} -yA ${s}" 2 IGNORE IGNORE

# Change the type flag
docommand v10 "${admin} -ftA ${s}" 0 IGNORE IGNORE

# Module flag match
docommand v11 "${vg_val} -yA ${s}" 0 IGNORE IGNORE

# SID not found
docommand v12 "${vg_val} -r1.2 ${s}" 4 IGNORE IGNORE

# SID not valid
docommand v13 "${vg_val} -r1.2xyzzy ${s}" 8 IGNORE IGNORE

chmod 0 "${s}" || abandon_test_script "Cannot change permissions for file ${s}"
# Cannot read file
docommand v14 "${vg_val} ${s}" 16 IGNORE IGNORE
chmod +r "${s}" || abandon_test_script "Cannot reset permissions for file ${s}"

# Missing file
docommand v15 "${vg_val} -r1.1" 128 IGNORE IGNORE

# Too many -r options
docommand v16 "${vg_val} -r1.1 -r1.2 ${s}" 64 IGNORE IGNORE


# A corrupt file
remove s.corrupt
cat valbasic.sh "${s}" > s.corrupt || abandon_test_script "cannot create file s.corrupt"
docommand v17 "${vg_val} -r1.1 s.corrupt" 32 IGNORE IGNORE
remove s.corrupt


# Too many -r options (a different way)
docommand v18 "${vg_val} -r1.1 -s -r1.1 ${s}" 64 IGNORE IGNORE

# Too many -m options
docommand v19 "${vg_val} -mX -mX ${s}" 64 IGNORE IGNORE

# Too many -y options
docommand v20 "${vg_val} -yX -yX ${s}" 64 IGNORE IGNORE

# Unknown option
docommand v21 "${vg_val} -X ${s}" 64 IGNORE IGNORE




# done rc   0 (success)
# done rc   1 (Val_MismatchedM)
# done rc   2 (Val_MismatchedY)
# done rc   4 (Val_NoSuchSID)
# done rc   8 (Val_InvalidSID)
# done rc  16 (Val_CannotOpenOrWrongFormat)
# done rc  32 (Val_CorruptFile)
# done rc  64 (Val_InvalidOption)
# done rc 128 (Val_MissingFile)

cleanup
success
