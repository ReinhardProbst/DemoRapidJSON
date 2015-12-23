bool GetValue(const rapidjson::Value &jval, const char *name, int &val)
{
 	const rapidjson::Value::ConstMemberIterator it = jval.FindMember(name);

	if(it == jval.MemberEnd() || !it->value.IsInt())
		return false;

	val = it->value.GetInt();

	return true;
}

bool GetValue(const rapidjson::Value &jval, const char *name, int *val)
{
 	const rapidjson::Value::ConstMemberIterator it = jval.FindMember(name);

	if(it == jval.MemberEnd() || !it->value.IsInt())
		return false;

	*val = it->value.GetInt();

	return true;
}

bool GetValue(const rapidjson::Value &jval, const char *name, bool &val)
{
 	const rapidjson::Value::ConstMemberIterator it = jval.FindMember(name);

	if(it == jval.MemberEnd() || !it->value.IsBool())
		return false;

	val = it->value.GetBool();

	return true;
}

bool GetValue(const rapidjson::Value &jval, const char *name, bool *val)
{
 	const rapidjson::Value::ConstMemberIterator it = jval.FindMember(name);

	if(it == jval.MemberEnd() || !it->value.IsBool())
		return false;

	*val = it->value.GetBool();

	return true;
}

bool GetValue(const rapidjson::Value &jval, const char *name, rapidjson::Value &val)
{
 	const rapidjson::Value::ConstMemberIterator it = jval.FindMember(name);

	if(it == jval.MemberEnd() || !it->value.IsObject())
		return false;

	val = const_cast<rapidjson::Value&>(it->value);

	return true;
}

bool GetValue(const rapidjson::Value &jval, const char *name, rapidjson::Value *val, const rapidjson::SizeType cnt, rapidjson::SizeType &n)
{
 	const rapidjson::Value::ConstMemberIterator it = jval.FindMember(name);

	if(it == jval.MemberEnd() || !it->value.IsArray())
		return false;

	n = 0;
	rapidjson::SizeType i = 0;

	for(rapidjson::Value::ConstValueIterator itv = it->value.Begin(); itv != it->value.End() && i < cnt; ++itv, ++i, ++n)
	{
		*val++ = const_cast<rapidjson::Value&>(*itv);
	}

	return true;
}
