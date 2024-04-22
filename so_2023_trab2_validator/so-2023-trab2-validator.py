#!/bin/bash
. /home/so/utils/bin/so_define_colours.sh
echo -e "${COLOUR_BOLD_LIGHTYELLOW}O seu script Validator não tinha um nome correto, mas foi corrigido."
echo -e "A partir de agora, use antes o nome so_2023_trab2_validator.py com as mesmas opções"
echo -e "Bom trabalho!!! Os docentes de SO ${COLOUR_NONE}"
[ -f "so-2023-trab2-validator.py" ] && ln -s /home/so/trabalho-2023-2024/utils/parte-2/so_2023_trab2_validator/so_2023_trab2_validator.py && rm -f so-2023-trab2-validator.py
