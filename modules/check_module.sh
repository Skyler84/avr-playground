offenders="$(avr-objdump -d $1 | grep -E '[^ir](call|jmp)')"
if [[ -z "$offenders" ]]; then
    echo "No offenders found in $1"
    exit 0
fi

echo "$offenders" | while IFS= read -r offender ; do  
    addr=$(echo "$offender" | awk '{print $1}' | sed 's/://')
    line=$(avr-addr2line -e $1 $addr)
    if [[ "$offender" =~ "call" ]]; then
        insn="call"
        echo "Found a call instruction: $offender"
        # Add your logic to handle call instructions here
    elif [[ "$offender" =~ "jmp" ]]; then
        insn="jmp"
        echo "Found a jump instruction: $offender"
        # Add your logic to handle jump instructions here
    fi
    echo "  $line"
done



exit 1