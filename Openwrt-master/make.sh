#!/bin/bash
#$1 should be p10 or p20

board=fullmask_h8898
imgtype=rel
tag_filter="sf21x2880_sdk" # Default tag filter

#$1 p10 p10m p20 86v clean default p10m
if [ -n "$1" ]; then
	board=$1
fi

#$2 shoule be auto/flash/rel/debug_1/debug_2  default rel
if [ -n "$2" ]; then
	imgtype=$2
fi

case ${board} in
fullmask_bpi)
	tag_filter="sf21h8898_gateway" # Default tag filter
	target_board=target/linux/siflower/sf21h8898_bpi_nand_def.config
	;;
fullmask_bpi-rv2-nand)
	tag_filter="sf21h8898_gateway"
	target_board=target/linux/siflower/sf21h8898_bpi-rv2-nand_def.config
	;;
fullmask_siflower-one-nand)
	tag_filter="sf21h8898_gateway" # Default tag filter
	target_board=target/linux/siflower/sf21h8898_siflower_one_nand_def.config
	;;
fullmask_siflower-one-nor)
	tag_filter="sf21h8898_gateway" # Default tag filter
	target_board=target/linux/siflower/sf21h8898_siflower_one_nor_def.config
	;;
fullmask_gw-bc-nand)
	tag_filter="sf21h8898_gateway" # Default tag filter
	target_board=target/linux/siflower/sf21h8898_gw-bc-nand_def.config
	;;
clean)
	echo "clean build enviroment"
	echo "delete build dir "
	rm -rf build_dir
	echo "delete tmp"
	rm -rf tmp
	echo "delete board"
	rm .board
	exit 1
	;;
*)
	echo "error arg"
	exit 1 # Command to come out of the program with status 1
	;;
esac

git branch >/dev/null 2>&1
if [ ! $? -eq 0 ]; then
	echo "does not exist git base"
	tag="0.0"
	branch="default"
else
	# branch=`git branch  -r | grep "\->" |awk -F "/" 'END{print $NF}'`
	local_branch_name=$(git branch -vv | grep "*" | awk '{print $2}')
	# echo "local branch name $local_branch_name"
	if [ "$local_branch_name" = "(no" ]; then
		echo "branch set fail no local branch"
		exit 1
	fi

	branch=$(git branch -vv | grep "*" | awk -F "[][]" '{print $2}' | awk -F "[/:]" '{print $2}')
	echo "branch is $branch"

	# handle release branch case release branch name is not release!!!
	is_rep_release=$(echo $branch | grep "release.rep")
	isrelease=$(echo $branch | grep "release")

	if [ "$is_rep_release" != "" ]; then
		branch="release.rep"
	elif [ "$isrelease" != "" ]; then
		branch="release"
	fi

	# Tag extraction optimization with tag_filter
	tags=$(git tag --merged $branch | grep "$tag_filter")
	if [ -z "$tags" ]; then
		echo "Warning: No matching tag found for filter: $tag_filter"
		tag="unknown"
		tag_commit="unknown"
	else
		# Extract the tags on the same commit and select the latest matching one by tag_filter
		latest_tag=$(for t in $tags; do
			commit_date=$(git log -1 --format='%at' refs/tags/$t)
			echo "$commit_date $t"
		done | sort -nr | grep "$tag_filter" | head -n 1 | awk '{print $2}')
		tag=$latest_tag
		tag_commit=$(git rev-list -n 1 $tag)
	fi
	version=$tag

	echo "Filtered tag for branch $branch is $tag filter $tag_filter"

	if [ -z "$version" ]; then
		version="unknown"
	fi
	echo "version is $version"

	# Update tag_commit based on the filtered tag
	short_tag_commit=$(echo "$tag_commit" | cut -c 1-7)
	echo "tag commit $short_tag_commit (short format)"
	last_commit=$(git rev-parse --short HEAD)
	echo "last commit $last_commit"

	if [ $short_tag_commit != ${last_commit:0:7} ] || [ -z "$version" ]; then
		commit_suffix=$last_commit
	fi
fi

echo "set up board $target_config"

# TODO: Theses config files do not exist
if [ "$imgtype" = "flash" ]; then
	target_board=target/linux/siflower/${chip}_p10_${chip_ver}_flash.config
fi

if [ -f .board ] && [ "$imagetype" != "auto" ]; then
	cmp_reselt=$(cmp $target_board .config)
	if [ -n "$cmp_reselt" ]; then
		echo "board change, clean build enviroment"
		rm -rf build_dir
		rm -rf tmp
		cp $target_board .config
	fi
else
	cp $target_board .config
fi

chip=$(grep "^CONFIG_TARGET_siflower_" .config | head -1 | awk -F "[_=]" '{print $4}')
chip_ver=fullmask

type=$(echo ${board} | awk -F "_" '{$1=""; print $0}' | sed "s/^ //g" | tr " " "_")

echo "build board is ${type} chip ${chip}_${chip_ver} imgtype $imgtype"

if [ -n "$commit_suffix" ] && [ -n "$version" ]; then
	prefix=openwrt_${branch}_${chip}_${chip_ver}_${type}_${imgtype}_${version}_${commit_suffix}
elif [ -n "$version" ]; then
	prefix=openwrt_${branch}_${chip}_${chip_ver}_${type}_${imgtype}_${version}
else
	prefix=openwrt_${branch}_${chip}_${chip_ver}_${type}_${imgtype}_${commit_suffix}
fi

target_bin="bin/targets/siflower/${chip}/openwrt-siflower-${chip}-${type}-squashfs-sysupgrade.bin"

echo "target_bin is ${target_bin}"

echo "prefix is ${prefix}"

#set up version.mk
rm include/siwifi_version.mk
echo "VERSION_DIST:=SiWiFi" >> include/siwifi_version.mk
echo 'VERSION_NICK:=$(PROFILE)' >> include/siwifi_version.mk
echo "VERSION_NUMBER:=${prefix}" >> include/siwifi_version.mk

sed -e '12cVERSION_NUMBER:='${prefix}'' include/siwifi_version.mk > tmp_version

sed -e '14cVERSION_NUMBER:='${prefix}'' tmp_version > include/siwifi_version.mk

rm tmp_version

echo "set openwrt version"

if [ "$imgtype" = "debug_1" ]; then
	sed -i "s/# CONFIG_KERNEL_KASAN is not set/CONFIG_KERNEL_KASAN=y/g" .config
	sed -i "s/# CONFIG_KERNEL_UBSAN is not set/CONFIG_KERNEL_UBSAN=y/g" .config
fi

if [ "$imgtype" = "debug_2" ]; then
	cmd="sed -i .config"
	for i in PROVE_LOCKING SOFTLOCKUP_DETECTOR DETECT_HUNG_TASK WQ_WATCHDOG \
		DEBUG_ATOMIC_SLEEP DEBUG_VM; do
		cmd="$cmd -e 's/# CONFIG_KERNEL_${i} is not set/CONFIG_KERNEL_${i}=y/g'"
	done
	eval $cmd
fi

rm $target_bin

case ${imgtype} in
auto)
	echo "CONFIG_PACKAGE_autotest=y" >>.config
	echo "build auto"
	;;

flash)
	echo "build flash"
	;;
rel)
	echo "build release"
	;;
debug_1)
	echo "build debug_1"
	;;
debug_2)
	echo "build debug_2"
	;;
*)
	echo "imgtype error arg"
	exit 1 # Command to come out of the program with status 1
	;;
esac

make defconfig
make package/base-files/clean
make package/network/services/hostapd/clean
make package/dpns/clean
make package/sf_genl_user/clean
make -j32 V=s

if [ -f $target_bin ]; then
	cp $target_bin ${prefix}.bin
else
	echo "build fail, don't get target_bin"
fi

echo "build end"
