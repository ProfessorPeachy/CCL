
;******************** Site Declaration Section ********************
;**
declare siteKey = c13 with constant("mauryregional")             ; Set your site key here
;declare fileLocation = c24 with constant("EXTRACTS:")       ; Set your File Output Location here
declare fileLocation = vc with constant(build("/cerner/d_",cnvtlower(curdomain),"/data/batch_files/ccl_extracts/protenus/")) 

call echo(build2("fileLocation in common script include " , fileLocation))
;declare fileLocation = c24 with constant("CCLUSERDIR:")       ; Set your File Output Location here

;********************  Logical Domain Section ********************
;*
declare logicalDomainID = f8 with noconstant(0.0)

;*** Ensure that the executing User is associated to the specified Logical Domain
;*
declare prsnlID = f8 with constant(validate(reqinfo->updt_id,(-1.0)))

if (prsnlID > 0)
  select into "nl:"
  from
    prsnl p
  plan p
    where p.person_id =	prsnlID
  head report
    logicalDomainID = p.logical_domain_id
  with nocounter
else
  set logicalDomainID = cnvtreal(pLogicalDomainID)
endif

if (logicalDomainID > 0)
    ;*** Used when LOGICAL_DOMAIN_ID exists on the table we are already accessing
    ;*
    declare PARSER_LOGICAL_DOMAIN_ID = vc with constant(
        build2(
            "TABLE_ALIAS.logical_domain_id = "
                ,logicalDomainID
        )
    ), protect

    ;*** Used when Sub-Selects are needed
    ;*
    declare PARSER_LOGICAL_DOMAIN_QUERY = vc with constant(
        build2(
            "exists (select 1 from person x where x.person_id = TABLE_ALIAS.person_id and x.logical_domain_id = "
                ,logicalDomainID
            ,")"
        )
    ), protect
else
    declare PARSER_LOGICAL_DOMAIN_ID = vc with constant("1 = 1"), protect
    declare PARSER_LOGICAL_DOMAIN_QUERY = vc with constant("1 = 1"), protect
endif

;******************** Code Value Declaration Section ********************
;**
declare cvActivityType_Pharmacy = f8 with constant(uar_get_code_by("DISPLAYKEY",106,"PHARMACY")),protect
declare cvCatalogType_Pharmacy = f8 with constant(uar_get_code_by("DISPLAYKEY",6000,"PHARMACY")),protect

declare cvNDC = f8 with constant(uar_get_code_by("DISPLAYKEY",11000,"NDC")),protect
declare cvMedIdentType_BrandName = f8 with constant(uar_get_code_by("DISPLAYKEY",11000,"BRANDNAME")),protect
declare cvMedIdentType_GenericName = f8 with constant(uar_get_code_by("DISPLAYKEY",11000,"GENERICNAME")),protect
declare cvMedIdentType_Description = f8 with constant(uar_get_code_by("DISPLAYKEY",11000,"DESCRIPTION")),protect
declare cvMedIdentType_InterfaceIdent1 = f8 with constant(uar_get_code_by("DISPLAYKEY",11000,"PYXIS")),protect
declare cvMedIdentType_InterfaceIdent2 = f8 with constant(uar_get_code_by("DISPLAYKEY",11000,"MCKESSONINTERFACEID")),protect
declare cvMedIdentType_InterfaceIdent3 = f8 with constant(uar_get_code_by("DISPLAYKEY",11000,"LAWSON")),protect
declare cvMedIdentType_InterfaceIdent4 = f8 with constant(0),protect
declare cvMedIdentType_InterfaceIdent5 = f8 with constant(0),protect
declare cvMedIdentType_InterfaceIdent6 = f8 with constant(0),protect

declare cvCensus = f8 with constant(uar_get_code_by("DISPLAYKEY",339,"CENSUS")),protect

declare cvPersonFreeText = f8 with constant(uar_get_code_by("DISPLAYKEY",302,"FREETEXT")),protect

declare cvEncAliasTypeMRN = f8 with constant(uar_get_code_by("DISPLAYKEY",319,"MRN")),protect
declare cvEncAliasTypeVisit = f8 with constant(uar_get_code_by("DISPLAYKEY",319,"FINNBR")),protect

declare cvAddressHome = f8 with constant(uar_get_code_by("DISPLAYKEY",212,"HOME")),protect
declare cvAddressWork = f8 with constant(uar_get_code_by("DISPLAYKEY",212,"BUSINESS")),protect

declare cvPhoneWork = f8 with constant(uar_get_code_by("DISPLAYKEY",43,"BUSINESS")),protect
declare cvPhoneHome = f8 with constant(uar_get_code_by("DISPLAYKEY",43,"HOME")),protect
declare cvPhoneMobile = f8 with constant(uar_get_code_by("DISPLAYKEY",43,"MOBILE")),protect

declare cvAliasSSN = f8 with constant(uar_get_code_by("DISPLAYKEY",4,"SSN")),protect
declare cvAliasMRN = f8 with constant(uar_get_code_by("DISPLAYKEY",4,"MRN")),protect

declare cvOrgReltnEmployer = f8 with constant(uar_get_code_by("DISPLAYKEY",338,"EMPLOYER")),protect

declare cvAliasNPI = f8 with constant(uar_get_code_by("DISPLAYKEY",320,"NATIONALPROVIDERIDENTIFIER")),protect
declare cvAliasPoolCdHrIdent = f8 with constant(695333)
declare cvAliasTypeCdHrIdent = f8 with constant(1091)

declare cvReltnSupervisor = f8 with constant(uar_get_code_by("DISPLAYKEY",375,"SUPERVISOR")),protect




;******************** File Name Section ********************
;*
;* Define Output List
;*
record rOutputFiles (
	1 cnt = i4
	1 data [*]
		2 name = vc
)
set rOutputFiles->cnt = 0

;* Determine Client Mnemonic
;*
declare clientMnemonic = vc with noconstant("")

select into "nl:"
from
  logical_domain ld
plan ld
  where ld.logical_domain_id = logicalDomainID
head report
  clientMnemonic = cnvtlower(trim(ld.mnemonic_key))
with nocounter

declare gfnFileName = vc with noconstant("")
declare gfnStartDate = dq8 with noconstant(curdate)
declare gfnEndDate = dq8 with noconstant(curdate)

declare getFileName(srFilePrefix = vc, srStartDate = vc, srEndDate = vc) = vc
subroutine getFileName(srFilePrefix, srStartDate, srEndDate)

  set gfnFileName = fileLocation

  ;*** Prepend the Client Mnemonic for CernerWorks Sites
  ;*
  if (trim(clientMnemonic) > "")
    set gfnFileName = build(trim(gfnFileName),trim(clientMnemonic), "-")
  endif

  ;*** Add the filename prefix
  ;*
  set gfnFileName = build(trim(gfnFileName), trim(srFilePrefix), "-")

  ;*** Add Date Part of Filename
  ;*
  set gfnStartDate = cnvtdatetime(cnvtdate2(srStartDate,"YYYYMMDD"),0)
  set gfnEndDate = cnvtdatetime(cnvtdate2(srEndDate,"YYYYMMDD"),0)

  if (datetimediff(cnvtdatetime(gfnEndDate),cnvtdatetime(gfnStartDate)) < 2.0)
    set gfnFileName = build(
      trim(gfnFileName)
      ,trim(format(cnvtdatetime(gfnStartDate),"yyyyMMdd;;d"))
    )
  else
    set gfnFileName = build(
      trim(gfnFileName)
      ,trim(format(cnvtdatetime(gfnStartDate),"yyyyMMdd;;d"))
      ,"-"
      ,trim(format(cnvtlookbehind("1,d",cnvtdatetime(gfnEndDate)),"yyyyMMdd;;d"))
    )
  endif

  set gfnFileName = build(trim(gfnFileName), ".csv")

  ;*** Save the Extract Name
  ;*
  set rOutputFiles->cnt = rOutputFiles->cnt + 1
  set status = alterlist(rOutputFiles->data,rOutputFiles->cnt)
  set rOutputFiles->data[rOutputFiles->cnt]->name = trim(gfnFileName)

  return (gfnFileName)

end ; subroutine

;******************** Email Declaration Section ********************
;*
;* Enable/Disable Email Notifications
;*
declare ENABLE_EMAIL = i2 with constant(false), protect ; Default Value - Email Notifications to Off
;declare ENABLE_EMAIL = i2 with constant(true), protect ; Optional Value - Emails Notifications On

;* Declare From/To Addresses (if Emailing is Enabled)
;*
declare to_email = vc with constant("someone@somewhere.com"), protect 	; Enter a valid To Address
declare from_email = vc with constant("someone@somewhere.com"), protect		; Enter a valid From Address

;*** Define Email Functions
;*
;* Email for Start of Extract Notification
;*
SUBROUTINE EMAIL_START(extractType)

  ;*** Ensure that Emailing is Enabled
  ;*
  if (ENABLE_EMAIL = false) RETURN endif

	set cStartTimeStamp = TIMESTAMPDIFF(SYSTIMESTAMP,CNVTTIMESTAMP("01-JAN-1970 00:00:00.0"),1,1) + 18000.0 ;5hrs to EST

	set email_start_body = build('{"title":"',siteKey,"-",CURPROG,'","text":"Start of Process"'
			,',"date_happened":"',cStartTimeStamp,'","host":"',CURDOMAIN,'"'
			,',"aggregation_key":"',siteKey,'-',CURDOMAIN,'"'
			,',"tags":["client_short_name:',siteKey,'","cernerextract:',extractType,'"]}')

	set email_start_subject = CONCAT(trim(siteKey)," - ",CURPROG)

	call uar_send_mail(to_email,email_start_subject,email_start_body,from_email,5,"IPM.NOTE")

/*

	set dcl_email_status = 0
	set dcl_email_command = fillstring(2000," ")
	set dcl_email_command = concat("echo '"
			,trim(email_start_body),"' | mailx -r Cerner.Ops@cerner.com -s '"
			,email_start_subject,"' ",to_email)
	set dcl_email_length = size(trim(dcl_email_command))
	call dcl (dcl_email_command,dcl_email_length,dcl_email_status)

*/

END

;* Email for End of Extract Notification
;*
SUBROUTINE EMAIL_END(extractType,recordCount,emailLastId)

    ;*** Ensure that Emailing is Enabled
    ;*
    if (ENABLE_EMAIL = false) RETURN endif

	set cEndTimeStamp = TIMESTAMPDIFF(SYSTIMESTAMP,CNVTTIMESTAMP("01-JAN-1970 00:00:00.0"),1,1) + 18000.0 ;5hrs to EST

	set email_end_body = build('{"title":"',siteKey,"-",CURPROG,'","text":"End of Process. Core record count is ~'
			,recordCount,'.  Last Id ~',emailLastId,'."'
			,',"date_happened":"',cEndTimeStamp,'","host":"',CURDOMAIN,'"'
			,',"aggregation_key":"',siteKey,'-',CURDOMAIN,'"'
			,',"tags":["client_short_name:',siteKey,'","cernerextract:',extractType,'"]}')

	set email_end_subject = CONCAT(trim(siteKey)," - ",CURPROG)

	call uar_send_mail(to_email,email_end_subject,email_end_body,from_email,5,"IPM.NOTE")

/*

	set dcl_email_status = 0
	set dcl_email_command = fillstring(2000," ")
	set dcl_email_command = concat("echo '"
			,trim(email_end_body),"' | mailx -r Cerner.Ops@cerner.com -s '"
			,email_end_subject,"' ",dcl_to_email)
	set dcl_email_length = size(trim(dcl_email_command))
	call dcl (dcl_email_command,dcl_email_length,dcl_email_status)

*/
END

;*** Force OPSView to return a Successful Status if the Reply Record Exists
;*
IF (VALIDATE(REPLY->STATUS_DATA->STATUS) = TRUE)
    SET REPLY->STATUS_DATA->STATUS = "S"
ENDIF

;******************** Explorer Menu Output Section ********************
;*
declare output_report(pOutDev = vc) = null

subroutine output_report(pOutDev)
  if (cnvtupper(pOutDev) = "NL:") return endif

  select into value(pOutDev)
  from
    dummyt d
  head report
    col 0 "Extracts..."
    row + 2

    for(zZ = 1 to rOutputFiles->cnt)
      col 5 rOutputFiles->data[zZ]->name
      row + 1
    endfor
    row + 1

    col 0 "created."
  with nocounter

end ;subroutine
