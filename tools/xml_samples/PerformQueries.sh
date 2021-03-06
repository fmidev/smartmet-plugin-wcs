#! /bin/sh

set -x

# # Example: $HOME/.smartmetrc
# TARGET_SERVER="data.fmi.fi"
# FMI_API_KEY="put_your_fmi-apikey_here"

smartmet_rc=${HOME}/.smartmetrc

# Default target server
server=data.fmi.fi

# read user defined parameters
if [ -f ${smartmet_rc} ]; then
  . ${smartmet_rc}
  if [ -z "${FMI_API_KEY}" ]; then
    echo -e "\nWARNING: FMI_API_KEY not found or is zero length in file ${smartmet_rc}\n"
    FMI_API_KEY_URI_SUFFIX=""
  else
    FMI_API_KEY_URI_SUFFIX="/fmi-apikey/${FMI_API_KEY}"
  fi
  if [ -n "${TARGET_SERVER}" ]; then
    server="${TARGET_SERVER}"
    echo "Using user defined target server ${server}."
  fi
else
  echo -e "\nWARNING: file ${smartmet_rc} not found.\n"
fi

# Use proxy if the script is not run on the same host as target server.
host_name=$(hostname);
if [ ! -z ${http_proxy} ] && [ "${host_name}" != "${server}" ]; then
  proxy="--proxy ${http_proxy}"
  echo "Using http_proxy: ${http_proxy}";
else
  proxy="";
  export http_proxy=$proxy
fi

files=$(cd request && ls -1b *.xml | sort)

rm -rf response
mkdir -p response

for req in $files; do
    curl ${proxy} \
         --fail \
         --output response/$req \
         --header "Content-Type: text/xml" \
         --data-binary "@request/$req" \
         http://$server${FMI_API_KEY_URI_SUFFIX}/wfs || exit 1
done

exit 0;
