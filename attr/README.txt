alias pretty-attributes="python devtree.py" (in bashrc file)

-------------------------------------------------------------

pretty-attributes -h
pretty-attributes -help

------------------------------------------------------------

(To view the output in JSON format, specify -j or -json)
pretty-attributes
pretty-attributes -json

pretty-attributes -limited
pretty-attributes -json -limited

pretty-attributes ATTR_PHYS_DEV_PATH,ATTR_FAPI_NAME
pretty-attributes -json ATTR_PHYS_DEV_PATH,ATTR_FAPI_NAME


-----------------------------------------------------------

(-n and -nodes can be used interchangeably)
pretty-attributes -nodes
pretty-attributes -nodes sys-0/node-0/proc-0/pauc-1

-------------------------------------------------------

(-t and -target can be used interchangeably)
pretty-attributes -target p10.tpm:k0:n0:s0:c0 -json
pretty-attributes -target p10.tpm:k0:n0:s0:c0 -json -limited
pretty-attributes -target p10.tpm:k0:n0:s0:c0 -json
ATTR_HWAS_STATE,ATTR_FAPI_NAME
----------------------------------------------------------------

(-p and -path can be used interchangeably)
pretty-attributes -path sys-0/node-0/proc-3/eq-7/fc-0/core-0 -json
pretty-attributes -path sys-0/node-0/proc-3/eq-7/fc-0/core-0 -json -limited
pretty-attributes -path sys-0/node-0/proc-3/eq-7/fc-0/core-0 -json
ATTR_HWAS_STATE,ATTR_FAPI_NAME

--------------------------------------------------------------

(-f and -find can be used interchangeably)
pretty-attributes -f core
pretty-attributes -f fc ATTR_HWAS_STATE.FUNCTIONAL=True

---------------------------------------------------------------

(Backward Compatability)
pretty-attributes export

--------------------------------------------------------------------
pretty-attributes -l
pretty-attributes -list

-----------------------------------------------------------
