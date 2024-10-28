import boto3
from botocore.client import Config
from datetime import timedelta

s3_client = boto3.client(
    's3',
    endpoint_url='http://minio_url',
    aws_access_key_id='access_key',
    aws_secret_access_key='<secret_key>',
    config=Config(signature_version='s3v4')
)

url = s3_client.generate_presigned_url(
    'get_object',
    Params={'Bucket': 'test', 'Key': 'test.jpg'},
    ExpiresIn=3600
)

print(url)