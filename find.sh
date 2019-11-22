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

# arg: line, file
# find caller func by input line and file.
func_name()
{
	local qline qfile
	local line func
	local ret
	local l r i fd_i

	qline=$1
	qfile=$2

	# special: judge if arg2(file) is not *.c
	echo "$qfile" | grep -q '.*\.c'
	if [ $? -ne 0 ]; then
		return 255
	fi
	# show all func name in file.
	ret=$(global -f $qfile |
		awk '{printf("%s %d\n", $1, $2)}')

	func=($(echo "$ret" | cut -d' ' -f1))
	line=($(echo "$ret" | cut -d' ' -f2))
	fd_i="-1"
	for ((i = 0; i < ${#line[*]}; ++i)); do
		local now_line

		now_line=${line[$i]}
		# find first line of { and } from funcname line.
		l=$(sed -n "$now_line"',${ /^{/ {=;q} }' $qfile)
		r=$(sed -n "$now_line"',${ /^}/ {=;q} }' $qfile)
		# in range is caller func, get the nearest one.
		if (( l <= $qline && $qline <= r )); then
			fd_i=$i
		fi
	done
	if [[ $fd_i != "-1" ]]; then
		echo "${func[$fd_i]} $qfile $qline"
	fi
}

# once query
query_main()
{
	local tmp

	preprocess
	# query pattern may be regex, so there may be many query string.
	for ((xl = 0; xl < $query_line_total; ++xl)); do
		tmp=$(func_name ${line_list[$xl]} ${file_list[$xl]})
		if [[ -n "$tmp" ]]; then
			printf '%40s %40s %20s %5d\n' ${query_name_list[$xl]} $tmp
		fi
	done
}

# just sort output from once query.
once_output()
{
	local ret

	# unique.
	ret=$(query_main)
	if [ -n "$ret" ]; then
		#ret=$(echo "$ret" | sort -k2,3 -u | sort -k3 -k2 -k1)
		ret=$(echo "$ret" | sort -k3 -k2 -k1 -u | awk '!cnt[$2" "$3]++{print}')
		# add line number prefix.
		ret=$(echo "$ret" | awk '{printf "%d %s\n", NR, $0}')
		echo "$ret"
	fi
}

# multiple query with key interaction.
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
