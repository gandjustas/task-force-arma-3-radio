#!/usr/bin/env python3

import os
import sys
import subprocess

made = 0
failed = 0
skipped = 0
removed = 0

def mod_time(path):
    if not os.path.isdir(path):
        return os.path.getmtime(path)
    maxi = os.path.getmtime(path)
    for p in os.listdir(path):
        maxi = max(mod_time(os.path.join(path, p)), maxi)
    return maxi


def check_for_changes(addonspath, module):
    if not os.path.exists(os.path.join(addonspath, "{}.pbo".format(module))):
        return True
    return mod_time(os.path.join(addonspath, module)) > mod_time(os.path.join(addonspath, "{}.pbo".format(module)))

def check_for_obsolete_pbos(addonspath, file):
    module = file[0:-4]
    if not os.path.exists(os.path.join(addonspath, module)):
        return True
    return False

def build(armake, addonspath, addon):
    global made, skipped, removed, failed
    path = os.path.join(addonspath, addon)
    if not os.path.isdir(path):
        return
    if addon[0] == ".":
        return
    if not check_for_changes(addonspath, addon):
        skipped += 1
        print("  Skipping {}.".format(addon))
        return

    print("# Making {} ...".format(addon))

    try:
        # if os.path.exists("{}\\$PBOPREFIX$".format(p)) or os.path.exists("{}\\$PBOPREFIX$.txt".format(p)):
        #     subprocess.check_output([
        #         "makepbo",
        #         "-JNUP",
        #         p,
        #         "{}.pbo".format(p)
        #     ], stderr=subprocess.STDOUT)
        # else:
        #     subprocess.check_output([
        #         "makepbo",
        #         "-JNUP",
        #         "-@={}".format(p),
        #         p,
        #         "{}.pbo".format(p)
        #     ], stderr=subprocess.STDOUT)
        subprocess.check_output([
            armake,
            "build",
            "-p",
            "--force",
            path,
            "{}.pbo".format(path)
        ])
    except Exception as e:
        failed += 1
        print("  Failed to make {}.".format(addon))
        print(e)
    else:
        made += 1
        print("  Successfully made {}.".format(addon)) 
    return

def main():
    global made, skipped, removed, failed
    
    print("""
  ####################
  # SGTu Debug Build #
  ####################
""")

    scriptpath = os.path.realpath(__file__)
    projectpath = os.path.dirname(os.path.dirname(scriptpath))
    addonspath = os.path.join(projectpath, "addons")

    armake =  os.path.join(os.path.dirname(scriptpath), "armake_w64" if (sys.maxsize > 2**32) else "armake_w32")

    for file in os.listdir(addonspath):
        if os.path.isfile(file):
            if check_for_obsolete_pbos(addonspath, file):
                removed += 1
                print("  Removing obsolete file => " + file)
                os.remove(file)
    print("")        
    
    for p in os.listdir(addonspath):
        build(armake, addonspath, p)

    serverpath = os.path.join(projectpath, "server")
    p = "cba_settings_userconfig"
    path = os.path.join(serverpath, p)
    build(armake, serverpath, p)

    print("\n# Done.")
    print("  Made {}, skipped {}, removed {}, failed to make {}.".format(made, skipped, removed, failed))


if __name__ == "__main__":
    sys.exit(main())
