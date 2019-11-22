#!/bin/bash
# use with preprocess by gtags, find func's caller func continuously.

# arg parse: func_pattern
if [ $# -ne 1 ]; then
	echo "input query func pattern"
	exit -1
fi

# arg maybe regex.
pattern=$1
call_path=

# current time by date
curtime() {
	echo "$(date +%s.%N)"
}

# debug
debug()
{
	echo "$1" >&2
}

# find caller pos for all matching patterns func.
preprocess()
{
	ret=$(global -xr $pattern |
		awk '{printf("%s %d %s\n", $1, $2, $3)}')

	query_name_list=($(echo "$ret" | cut -d' ' -f1))
	line_list=($(echo "$ret" | cut -d' ' -f2))
	file_list=($(echo "$ret" | cut -d' ' -f3))
	query_line_total=${#line_list[*]}
}

# We get info about below command:
# As a file and func info:
# 	global -f <file>.
# As a caller func and line info:
# 	global -xr <func>.
# This func will find which function does the line belong to(in a file).
# Args: (line)
func_name()
{
	local qline qfile
	local stline func_list line_list
	local l r i fd_i

	qline=$1
	qfile=$2

	# show all func name in file.
	#let $global_f=$(global -f $qfile | awk '{printf("%s %d\n", $1, $2)}')
	stline=$last_fd
	func_list=($(echo "$global_f" | cut -d' ' -f1))
	line_list=($(echo "$global_f" | cut -d' ' -f2))
	#echo "stline: $stline"
	#echo "func_list: ${func_list[*]}"
	#echo "line_list: ${line_list[*]}"
	#echo "line_list len: ${#line_list[*]}"
	fd_i=-1
	q_funcname=
	for ((i = stline; i < ${#line_list[*]}; ++i)); do
		local now_line

		now_line=${line_list[$i]}
		# find first line of { and } from funcname line.
		l=$(sed -n "$now_line"',$ {/) *{$\|^{$/ {=;q}}' $qfile)
		r=$(sed -n "$now_line"',${/^}$/ {=;q}}' $qfile)
		# in range is caller func, get the nearest one.
		if (( l <= qline && qline <= r )); then
			fd_i=$i
			last_fd=$i
		fi
		if [[ $fd_i != "-1" ]] && (( qline < l || r < qline )); then
			break 1
		fi
	done
	if [[ $fd_i != "-1" ]]; then
		q_funcname=${func_list[$fd_i]}
	fi
}

# once query
query_main()
{
	local tmp
	local pre_filename now_filename
	local last_fd global_f
	local qfile qline q_funcname

	#debug "preprocess begin: $(curtime)"
	preprocess
	#debug "preprocess end: $(curtime)"
	# query pattern may be regex, so there may be many query string.
	#debug "func name begin: $(curtime)"
	pre_filename=""

	for ((xl = 0; xl < $query_line_total; ++xl)); do
		#debug "call once begin: $(curtime)"
		now_filename=${file_list[$xl]}
		qfile=$now_filename
		qline=${line_list[$xl]}

		# Filter the file name in: *.c.
		echo "$now_filename" | grep -q '.*\.c'
		if [ $? -ne 0 ]; then
			continue 1
		fi
		if [[ "$now_filename" != "$pre_filename" ]]; then
			last_fd=0
			global_f=$(global -f ${file_list[$xl]} |
				awk '{printf("%s %d\n", $1, $2)}')
		fi
		pre_filename=$now_filename
		func_name $qline $qfile
		if [[ -n "$q_funcname" ]]; then
			printf '%40s %40s %20s %5d\n' ${query_name_list[$xl]} $q_funcname $qfile $qline
		fi
		#debug "call once end: $(curtime)"
	done
	#debug "func name end: $(curtime)"
}

# just sort output from once query.
once_output()
{
	local ret

	# unique.
	ret=$(query_main)
	#debug "sort begin: $(curtime)"
	if [ -n "$ret" ]; then
		#ret=$(echo "$ret" | sort -k2,3 -u | sort -k3 -k2 -k1)
		ret=$(echo "$ret" | sort -k3 -k2 -k1 -u | awk '!cnt[$2" "$3]++{print}')
		# add line number prefix.
		ret=$(echo "$ret" | awk '{printf "%d %s\n", NR, $0}')
		echo "$ret"
	fi
	#debug "sort end: $(curtime)"
}

# multiple query with key interaction.
# This func is mt mode. I rewrite it by cpp, at mode, show all call path.
multi_query()
{
	local reftable last_pattern
	local down up up_file up_line first first_pattern
	local qnum

	first=1
	first_pattern=
	for ((;;)); do
		if [ -z "$pattern" ]; then
			# quit.
			echo 'QAQ: quit.'
			if [[ -n "$call_path" ]]; then
				echo "$call_path" | less
			fi
			return 0
		elif [ "$last_pattern" = "$pattern" ]; then
			# just show last output.
			echo "$reftable" | less
		else
			# new query.
			reftable=$(once_output)
			echo "$reftable" | less
		fi
		last_pattern="$pattern"
		pattern=""
		# input branch for next query.
		read qnum
		echo $qnum | grep '[0-9]' -q
		if [ $? -eq 0 ]; then
			# new query pattern.
			# choose the caller func from current output
			# as next querry pattern.
			# now can recording the call path.
			down=$(echo "$reftable" | sed -n ${qnum}p | awk '{print $2}')
			pattern=$(echo "$reftable" | sed -n ${qnum}p | awk '{print $3}')
			up_file=$(echo "$reftable" | sed -n ${qnum}p | awk '{print $4}')
			up_line=$(echo "$reftable" | sed -n ${qnum}p | awk '{print $5}')
			up="$up_file:$up_line:$pattern"
			if ((first == 1)); then
				first=0
				first_pattern=$down
				call_path=$(echo -en "$first_pattern\n$up")
			else
				call_path=$(echo -en "$call_path\n$up")
			fi
		elif [[ "$qnum" = 'q' || "$qnum" = 'Q' ]]; then
			pattern=""
		else
			pattern=$last_pattern
		fi
	done
}

# choose once or multi query.
#once_output
#multi_query

# this just for test(debug).
query_main
