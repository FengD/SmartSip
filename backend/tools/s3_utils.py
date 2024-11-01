import boto3
from botocore.client import Config
from datetime import timedelta
import os


def get_image_s3_url(image_name):
    access_key = os.environ.get("MINIO_ACCESS_KEY")
    secret_key = os.environ.get("MINIO_SECRET_KEY")
    endpoint_url = os.environ.get("MINIO_ENDPOINT_URL")
    s3_client = boto3.client(
        's3',
        endpoint_url=endpoint_url,
        aws_access_key_id=access_key,
        aws_secret_access_key=secret_key,
        config=Config(signature_version='s3v4')
    )

    try:
        url = s3_client.generate_presigned_url(
            'get_object',
            Params={'Bucket': 'test', 'Key': image_name},
            ExpiresIn=3600
        )
        return url
    except Exception as e:
        print(f"Error generating URL: {e}")
        return None

if __name__ == "__main__":
    print(get_image_s3_url("image_20241025_164659.jpg"))
