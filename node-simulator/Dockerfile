FROM python:3.12-slim
WORKDIR /app
COPY simulator.py .
# -u: salida sin búfer, para ver cada mensaje en `docker compose logs`.
CMD ["python", "-u", "simulator.py"]
