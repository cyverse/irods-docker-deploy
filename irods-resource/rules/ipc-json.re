# This is a collection of functions and rules for constructing serialized JSON 
# documents. It does not support arrays yet.
#
# © 2021 The Arizona Board of Regents on behalf of The University of Arizona. 
# For license information, see https://cyverse.org/license.

_ipcJson_accumEncodedList(*Base, *SerialElmts) =
  if size(*SerialElmts) == 0 then *Base
  else 
	  let *prefix = if *Base == '' then '' else *Base ++ ',' in
    _ipcJson_accumEncodedList(*prefix ++ hd(*SerialElmts), tl(*SerialElmts))

_ipcJson_encodeObject(*SerialFields) = '{' ++ _ipcJson_accumEncodedList('', *SerialFields) ++ '}'

_ipcJson_encodeString(*Str) =
  let *escStr = '' in
  let *len = strlen(*Str) in
  let *pos = 0 in
  let *_ = while (*len > *pos) {
    let *c = substr(*Str, *pos, *pos + 1) in
    let *escC = 
      if *c == '"' then '\\"' else
      if *c == '\t' then '\\t' else
      if *c == '\n' then '\\n' else
      if *c == '\r' then '\\r' else
      if *c == '\\' then '\\\\' else
      *c
    in *escStr = *escStr ++ *escC;
    *pos = *pos + 1; }
  in '"' ++ *escStr ++ '"'

_ipcJson_mkField(*Label, *SerialVal) = '"' ++ *Label ++ '":' ++ *SerialVal

# construct a serialized JSON object field from a Boolean value
#
# Parameters:
#   *Label - the name of the object field
#   *Val - the Boolean value of the field.
#
ipcJson_boolean: string * boolean -> string
ipcJson_boolean(*Label, *Val) = _ipcJson_mkField(*Label, if *Val then 'true' else 'false')

# construct a serialized JSON document from its serialized fields
#
# Parameters:
#   *SerialFields - A list of pre-serialized fields to include in the document
#
ipcJson_document: list string -> string
ipcJson_document(*SerialFields) = _ipcJson_encodeObject(*SerialFields)

# construct a serialized JSON number field. This version performs no error checking.
#
# Parameters:
#   *Label - the name of the field
#   *Val - the value of the field
#
ipcJson_number: string * double -> string
ipcJson_number(*Label, *Val) = _ipcJson_mkField(*Label, '*Val')

# construct a serialized JSON object field from its serialized fields
#
# Parameters:
#   *Label - the name of the object field
#   *SerialFields - the list of pre-serialized fields ot include in the object
#
ipcJson_object: string * list string -> string
ipcJson_object(*Label, *SerialFields) = _ipcJson_mkField(
  *Label, _ipcJson_encodeObject(*SerialFields) )

# construct a serialized JSON string field
#
# Parameters:
#   *Label - the name of the field
#   *Val - the value of the field
#
ipcJson_string: string * string -> string
ipcJson_string(*Label, *Val) = _ipcJson_mkField(*Label, _ipcJson_encodeString(*Val))
