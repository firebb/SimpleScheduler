
int main() {
  int total_batch = 100;
  int batch_size = 10000000;
  for (int i = 0; i < total_batch; i++) {
    double *input1 = new double[batch_size];
    double *result1 = new double[batch_size];
    for (int j = 0; j < batch_size; j++) {
      result1[j] = 2 * input1[j];
    }
    delete[] input1;
    double *input2 = new double[batch_size];
    double *result2 = new double[batch_size];
    for (int j = 0; j < batch_size; j++) {
      result2[j] = 2 *input2[j];
    }
    delete[] input2;
    double *result3 = new double[batch_size];
    for (int j = 0; j < batch_size; j++) {
      result3[j] = result1[j] / result2[j];
    }
    delete[] result1;
    delete[] result2;
    delete[] result3;
  }
}
