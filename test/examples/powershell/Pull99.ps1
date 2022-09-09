# End simple string at closing quote or EOL.

$variable = 'test'
function test
{
}

$variable = 'test
function test
{
}

$variable = "test"
function test
{
}

$variable = "test
function test
{
}
