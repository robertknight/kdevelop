
#This is the proper cmake macro for installing kde4 icons, but it needs 
#proper naming of the icons: hi<size>_<category>_<name>.png
kde4_install_icons(${ICON_INSTALL_DIR} )

########### install files ###############

install(FILES class.png enum.png field.png function.png namespace.png
    private_class.png private_enum.png private_field.png private_function.png private_struct.png private_union.png
    protected_class.png protected_enum.png protected_field.png protected_function.png protected_struct.png protected_union.png
    signal.png slot.png struct.png typedef.png union.png CTchildren.png CTclients.png CTparents.png CTdisconnected_parents.png CTsuppliers.png CTvirtuals.png CVclass.png CVglobal_meth.png
    CVglobal_var.png CVnamespace.png CVprivate_meth.png CVprivate_signal.png CVprivate_slot.png CVprivate_var.png
    CVprotected_meth.png CVprotected_signal.png CVprotected_slot.png CVprotected_var.png CVpublic_meth.png
    CVpublic_signal.png CVpublic_slot.png CVpublic_var.png CVstruct.png CVpackage_meth.png CVpackage_var.png CVtypedef.png
    DESTINATION ${DATA_INSTALL_DIR}/kdevelop/pics )


