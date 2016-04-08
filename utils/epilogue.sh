#!/bin/bash
#
# script that can be passed to pbs as epilogue.user. The cleanup
# after a job can be done here. Based on pbs documentation the
# argv are:
# argv[ 1] job id
# argv[ 2] job execution user name
# argv[ 3] job execution group name
# argv[ 4] job name
# argv[ 5] session id
# argv[ 6] list of requested resource limits
# argv[ 7] list of resources used by job
# argv[ 8] job execution queue
# argv[ 9] job account
# argv[10] job exit code
#
#

echo "########################## EPILOGUE SCRIPT ###########################"
JOBNAME=$4
RESOURCE_LIM=$6
RESOURCES=$7
EXIT=${10}
JOBDIR=${PBS_O_WORKDIR%/}

#- time stamp
STAMP=`date +%m%d.%H%M%S`

#- print and check resource usage
echo "Requested resources: ${RESOURCE_LIM}"
echo "Used resources     : ${RESOURCES}"

get_mem () {
    re="mem=([^,]+)(,|$)"
    if [[ $1 =~ $re ]]; then
	STR=${BASH_REMATCH[1]}
    fi

    re='([0-9]+)([KMGkmg])[bB]'
    if [[ $STR =~ $re ]]; then
	MEM=${BASH_REMATCH[1]}
	UNIT=${BASH_REMATCH[2]}
    fi

    kilo=1000
    shopt -s nocasematch
    case "$UNIT" in
	m) MEM=$((${MEM}*${kilo}));;
	g) MEM=$((${MEM}*${kilo}))
	   MEM=$((${MEM}*${kilo}))
	    ;;
    esac

    echo $MEM
}

get_wct () {
    re="walltime=([^,]+)(,|$)"
    if [[ $1 =~ $re ]]; then
	STR=${BASH_REMATCH[1]}
    fi
    echo $(date -d $STR +"%s")
}

MEML=$(get_mem ${RESOURCE_LIM})
MEM=$(get_mem ${RESOURCES})
WCTL=$(get_wct ${RESOURCE_LIM})
WCT=$(get_wct ${RESOURCES})

if [[ $MEM -gt $MEML ]]; then
    echo "Script exceeded its memory limit"
    STAMP="${STAMP}.mem"
elif [[ $WCT -gt $WCTL ]]; then
    echo "Script exceeded its walltime"
    STAMP="${STAMP}.wct"
elif [[ $EXIT -ne 0 ]]; then
    echo "Script exited with non-zero code"
    STAMP="${STAMP}.${EXIT}.trm"
else
    STAMP="${STAMP}.fin"
fi

#- mark dir as done
if [[ ${JOBDIR} =~ "scratch" ]]; then
    JOBDIRDIR=$(dirname ${JOBDIR})
    NEWNAME=$(basename ${JOBDIR})
    NEWNAME="${JOBDIRDIR}/comp.${NEWNAME}.${STAMP}"
    echo "moving job from '${JOBDIR}' to '${NEWNAME}'"
    mv ${JOBDIR} ${NEWNAME}
fi

exit 0
