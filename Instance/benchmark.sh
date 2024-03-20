#!/bin/bash
SIMULATOR="Simulator.jar"
SOLVER="../bin/NurseRostering"

CURRENT_DIR="$PWD"

cd "$(dirname "$0")"
echo "Current directory: $PWD"

problem=$1
if [ -z "$problem" ]; then
    echo "No output directory specified."
    echo "Usage: ./benchmarkW4.sh <problem> eg. ./benchmarkW4.sh n005w4_0_1-2-3-3"
    exit 1
fi

WORKING_DIR="$PWD"

instance=$(echo $problem | cut -d'_' -f1)
h0=$(echo $problem | cut -d'_' -f2)
w=$(echo $problem | cut -d'_' -f3)
# w="1-2-3-3" or w="1-2-3-5-6-4-5-3"
# split w into an array of each week
IFS='-' read -r -a w <<< "$w"

outputDir="output"
timeout=10.5
rand=8787

# outputDir
read -p "Enter outputDir [$outputDir]: " input_outputDir
outputDir=${input_outputDir:-$outputDir}
outputDir=$CURRENT_DIR/$outputDir
echo "Output directory is relative to the directory: $CURRENT_DIR"
echo "The absolute path of the output directory is: $outputDir"
if [ ${#w[@]} -eq 4 ]; then  
    outputDir="${outputDir}/${instance}/Solution_H_${h0}-WD_${w[0]}-${w[1]}-${w[2]}-${w[3]}"
elif [ ${#w[@]} -eq 8 ]; then
    outputDir="${outputDir}/${instance}/Solution_H_${h0}-WD_${w[0]}-${w[1]}-${w[2]}-${w[3]}-${w[4]}-${w[5]}-${w[6]}-${w[7]}"
else
    echo "Invalid number of weeks."
    exit 1
fi  

if [ -d ${outputDir} ]; then
    rm -rf ${outputDir}
    echo "Delete the existing ${outputDir} directory"
fi

echo "create ${outputDir} directory"
mkdir -p ${outputDir}

read -p "Enter timeout [$timeout]: " input_timeout
timeout=${input_timeout:-$timeout}

read -p "Enter rand [$rand]: " input_rand
rand=${input_rand:-$rand}

echo ""
echo "Instance: $instance"
echo "h0: $h0"
echo "w: $w"
echo "outputDir: $outputDir"
echo "timeout: $timeout"
echo "rand: $rand"
echo "Simulator: ${SIMULATOR}"
echo "Solver: ${SOLVER}"
echo ""
echo "Calling the ${SIMULATOR}...."
echo "Current directory: $PWD"
echo ""
if [ ${#w[@]} -eq 4 ]; then
    echo "java -jar ${SIMULATOR} \\
    --sce ${instance}/Sc-${instance}.txt \\
    --his ${instance}/H0-${instance}-${h0}.txt \\
    --weeks ${instance}/WD-${instance}-${w[0]}.txt ${instance}/WD-${instance}-${w[1]}.txt ${instance}/WD-${instance}-${w[2]}.txt ${instance}/WD-${instance}-${w[3]}.txt \\
    --solver ${SOLVER} \\
    --outDir ${outputDir}/ \\
    --timeout ${timeout} \\
    --cus \\
    --runDir ./ \\
    --rand ${rand}"

    echo ""

    read -rp "Do you want to continue? (y/N): " choice  # capitalize N to show it's the default value
    case "${choice}" in
        [Yy]|[Yy][Ee][Ss]|"" )  # allow for 'yes' or empty input
            echo "User chose to continue." ;;
        [Nn]|[Nn][Oo] )
            echo "User chose to cancel."
            exit ;;
        * )  # handle invalid input
            echo "Invalid choice. Please enter 'y' or 'n'."
            exit 1 ;;
    esac
    echo ""


    java -jar ${SIMULATOR} \
    --sce ${instance}/Sc-${instance}.txt \
    --his ${instance}/H0-${instance}-${h0}.txt \
    --weeks ${instance}/WD-${instance}-${w[0]}.txt ${instance}/WD-${instance}-${w[1]}.txt ${instance}/WD-${instance}-${w[2]}.txt ${instance}/WD-${instance}-${w[3]}.txt \
    --solver ${SOLVER} \
    --outDir ${outputDir}/ \
    --timeout ${timeout} \
    --cus \
    --runDir ./ \
    --rand ${rand} 
elif [ ${#w[@]} -eq 8 ]; then
    echo "java -jar ${SIMULATOR} \\
    --sce ${instance}/Sc-${instance}.txt \\
    --his ${instance}/H0-${instance}-${h0}.txt \\
    --weeks ${instance}/WD-${instance}-${w[0]}.txt ${instance}/WD-${instance}-${w[1]}.txt ${instance}/WD-${instance}-${w[2]}.txt ${instance}/WD-${instance}-${w[3]}.txt ${instance}/WD-${instance}-${w[4]}.txt ${instance}/WD-${instance}-${w[5]}.txt ${instance}/WD-${instance}-${w[6]}.txt ${instance}/WD-${instance}-${w[7]}.txt \\
    --solver ${SOLVER} \\
    --outDir ${outputDir}/ \\
    --timeout ${timeout} \\
    --cus \\
    --runDir ./ \\
    --rand ${rand}"

    echo ""

    read -rp "Do you want to continue? (y/N): " choice  # capitalize N to show it's the default value
    case "${choice}" in
        [Yy]|[Yy][Ee][Ss]|"" )  # allow for 'yes' or empty input
            echo "User chose to continue." ;;
        [Nn]|[Nn][Oo] )
            echo "User chose to cancel."
            exit ;;
        * )  # handle invalid input
            echo "Invalid choice. Please enter 'y' or 'n'."
            exit 1 ;;
    esac
    echo ""

    java -jar ${SIMULATOR} \
    --sce ${instance}/Sc-${instance}.txt \
    --his ${instance}/H0-${instance}-${h0}.txt \
    --weeks ${instance}/WD-${instance}-${w[0]}.txt ${instance}/WD-${instance}-${w[1]}.txt ${instance}/WD-${instance}-${w[2]}.txt ${instance}/WD-${instance}-${w[3]}.txt ${instance}/WD-${instance}-${w[4]}.txt ${instance}/WD-${instance}-${w[5]}.txt ${instance}/WD-${instance}-${w[6]}.txt ${instance}/WD-${instance}-${w[7]}.txt \
    --solver ${SOLVER} \
    --outDir ${outputDir}/ \
    --timeout ${timeout} \
    --cus \
    --runDir ./ \
    --rand ${rand}
else
    echo "Invalid number of weeks."
    exit 1
fi