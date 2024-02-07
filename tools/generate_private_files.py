from pathlib import Path
from uuid import uuid4

private_dir = Path('../private')

id = uuid4()
id_str = str(id)
#id_str = id.hex
print('Saving new UUID: {}'.format(id_str))

with open(private_dir/'uuid4.txt', mode='wt') as f:
    f.write(id_str)
