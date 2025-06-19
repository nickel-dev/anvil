import sys, os

if len(sys.argv) < 2:
    print("error: no input folder given!")
    exit(0)

source = os.listdir(sys.argv[1])

for i in source:
    n = i.split('.')
    if n[-1] != 'glsl':
        continue

    name = n[0]
    file = ''

    with open(os.path.join(sys.argv[1], i), 'r') as f:
        file = f.read().replace('\n', '\\n')

    path = os.path.join(sys.argv[1], i) + '.h'
    with open(path, 'w') as f:
        f.write(f"#ifndef __SHADER_SOURCE_{name.upper()}_H_\n#define __SHADER_SOURCE_{name.upper()}_H_\nstatic const char *__shader_source_{name} = \"{file}\";\n#endif")
        print("shader compiled:", os.path.basename(path))