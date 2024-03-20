#!/bin/bash

CURRENT_DIR="$PWD"

cd "$(dirname "$0")"
echo "Current directory: $PWD"


outputDir=$1 # 1-n005w4_0_1-2-3-3 or 1-n005w8_0_1-2-3-5-6-4-5-3
isAutoExecuted=$2 # --yes or -y
verbose=$3 # --verbose or -v
if [ -z "$outputDir" ]; then
    echo "No output directory specified."
    echo "Usage: ./validatorW4.sh <outputDir> [--yes|-y] [--verbose|-v]"
    exit 1
elif [ ! -z "$isAutoExecuted" ] && [ "$isAutoExecuted" != "--yes" ] && [ "$isAutoExecuted" != "-y" ]; then
    echo "Invalid argument: $isAutoExecuted"
    echo "Usage: ./validatorW4.sh <outputDir> [--yes|-y] [--verbose|-v]"
    exit 1
elif [ ! -z "$isAutoExecuted" ] && [ "$isAutoExecuted" == "--yes" ] || [ "$isAutoExecuted" == "-y" ]; then
    echo "Auto-executed mode."
    echo "Output directory: $outputDir"
    isAutoExecuted=true
fi

# outputDir="output0/1-n005w4_0_1-2-3-3" or outputDir="output0/1-n005w8_0_1-2-3-5-6-4-5-3"
instance=$(basename "$outputDir" | cut -d'_' -f1 | cut -d'-' -f2)
h0=$(basename "$outputDir" | cut -d'_' -f2)
w=$(basename "$outputDir" | cut -d'_' -f3)

echo "Instance: $instance"
echo "h0: $h0"
echo "w: $w"

if [[ $isAutoExecuted != true ]]; then
    read -p "Press any key to continue"
fi

# w="1-2-3-3" or w="1-2-3-5-6-4-5-3"
# split w into an array of each week
IFS='-' read -r -a w <<< "$w"


outputDir=$CURRENT_DIR/$outputDir
echo "The absolute path of the output directory is: $outputDir"

if [ ${#w[@]} -eq 4 ]; then  
    weeks_list="${instance}/WD-${instance}-${w[0]}.txt ${instance}/WD-${instance}-${w[1]}.txt ${instance}/WD-${instance}-${w[2]}.txt ${instance}/WD-${instance}-${w[3]}.txt"
    sols_list="${outputDir}/sol-week0.txt ${outputDir}/sol-week1.txt ${outputDir}/sol-week2.txt ${outputDir}/sol-week3.txt"

elif [ ${#w[@]} -eq 8 ]; then
    weeks_list="${instance}/WD-${instance}-${w[0]}.txt ${instance}/WD-${instance}-${w[1]}.txt ${instance}/WD-${instance}-${w[2]}.txt ${instance}/WD-${instance}-${w[3]}.txt ${instance}/WD-${instance}-${w[4]}.txt ${instance}/WD-${instance}-${w[5]}.txt ${instance}/WD-${instance}-${w[6]}.txt ${instance}/WD-${instance}-${w[7]}.txt"
    sols_list="${outputDir}/sol-week0.txt ${outputDir}/sol-week1.txt ${outputDir}/sol-week2.txt ${outputDir}/sol-week3.txt ${outputDir}/sol-week4.txt ${outputDir}/sol-week5.txt ${outputDir}/sol-week6.txt ${outputDir}/sol-week7.txt"

else
    echo "Invalid w: $w"
    exit 1

fi

validator_command="java -jar validator.jar \\
    --sce \"$instance/Sc-$instance.txt\" \\
    --his \"$instance/H0-$instance-$h0.txt\" \\
    --weeks $weeks_list \\
    --sols $sols_list \\
    --out \"$outputDir/Validator-results.txt\" "

if [ -n "$verbose" ]; then
    validator_command+="--verbose"
fi


echo "$validator_command"
echo ""

if [[ $isAutoExecuted != true ]]; then
    read -rp "Do you want to continue? (y/n) [y]: " choice
    choice=${choice:-y}  # set default value to 'n' if user presses enter without input
    while true; do
        case ${choice} in
            [Yy]* ) # handle 'y' and 'yes' cases
                echo "User chose to continue."
                break ;;
            [Nn]* ) # handle 'n' and 'no' cases
                echo "User chose to cancel."
                exit ;;
            * ) # handle invalid input
                read -rp "Invalid choice. Please enter 'y' or 'n': " choice ;;
        esac
    done
fi

eval "$validator_command"
